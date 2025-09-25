/* RIO 包全名为 Robust IO 函数包。
 * 包中函数是对 Linux 基本 I/O 函数的封装，
 * 使其更加健壮、高效，更适用于网络编程。
 * 具体来说，它会自动处理读写中的不足值情况。
 * 这种情况在网络应用中经常出现，
 * 因此编写网络程序时，我们经常用到它。
 */
/* CSAPP 中实现的 RIO 提供了两类不同的函数： */
/* **无缓冲的 I/O 函数**
 * 无应用级的缓冲，对二进制数据读写到网络
 * 和从网络读写到二进制数据尤为有用。
 *
 * **有缓冲的 I/O 函数**
 * 从文本读取文本行和二进制，并会被缓存在应用级缓冲区中，
 * 该缓冲区是线性安全的。即同一个描述符上可以被交错地调用。
 */

/************************************************************/

#include "inc.h"

#define RIO_BUFSIZE 8192

// 无缓冲区的 I/O 函数
/* 直接在内存和文件之间传送数据，没有应用级缓冲。
 * 将二进制数据读写到网络和从网络读写二进制数据。
 */
ssize_t rio_readn(int fd, void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while(nleft > 0) {
        if((nread = read(fd, bufp, nleft)) < 0) {
            if(errno == EINTR) {    // 被信号处理程序的返回中断
                nread = 0;          // 重新调用 read()
            } else {
                return -1;          // read() 出错
            }
        }
        else if(nread == 0) {
            break;                  // EOF
        }
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);             // 返回读到的字节数
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while(nleft > 0) {
        if((nwritten = write(fd, bufp, nleft)) <= 0) {
            if(errno == EINTR) {
                nwritten = 0;
            } else {
                return -1;
            }
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;   // 不出错时一定会全部写入
}


// 带缓冲的输入函数
/* 从文件中读取文本行和二进制数据，
 * 文件的内容缓存在应用级缓冲区内。
 * 线程安全，在同一个描述符上可以被交错地调用
 */
// 一个类型为 rio_t 的读缓冲区
typedef struct {
    int rio_fd;                 // 与该缓冲区匹配的文件描述符
    int rio_cnt;                // 缓冲区中未读的字节数
    char *rio_bufptr;           // 指向缓冲区中第一个未读字节
    char rio_buf[RIO_BUFSIZE];  // 缓冲区
} rio_t;

// 初始化缓冲区, 将描述符 fd 与地址 rp 处的读缓冲区联系起来
void rio_readinitb(rio_t *rp, int fd) {
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

// rio_read 函数是 RIO 包都程序的核心，Linux read 函数的带缓冲版本
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n) {
    int cnt;
    // 先将描述符的内容读到缓冲区, 再从缓冲区读到 usrbuf
    while(rp->rio_cnt <= 0) {       // 缓冲区空时填充
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        if(rp->rio_cnt < 0) {
            if(errno != EINTR) {    // 被信号处理程序的返回打断则重新调用read
                return -1;
            }
        }
        else if(rp->rio_cnt == 0) { // EOF
            return 0;
        }
        else {
            rp->rio_bufptr = rp->rio_buf;
        }
    }

    cnt = n;
    if(rp->rio_cnt < n) {
        cnt = rp->rio_cnt;
    }
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}

/* rio_readlineb 从读缓冲区(rp处)复制一个文本行(包括换行符)到usrbuf，
 * 并用 NULL 字符结束这个文本行, 最多读 maxlen-1 个字节(给NULL留一个字节)。
 * 当缓冲区变空时会自动调用read重新填满缓冲区
 */
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) {
    int n, rc;
    char c, *bufp = usrbuf;

    for(n = 1; n < maxlen; n++) {
        if((rc = rio_read(rp, &c, 1)) == 1) {
            *bufp++ = c;    // 等价于 *bufp = c; bufp++;
            if(c == '\n') {
                n++;
                break;
            }
        } else if(rc == 0) {
            if(n == 1){
                return 0;
            } else {
                break;
            }
        } else {
            return -1;
        }
    }
    *bufp = 0;
    return n-1;
}

ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while(nleft > 0) {
        if((nread = rio_read(rp, bufp, nleft)) < 0) {
            return -1;
        }
        else if(nread == 0) {   // EOF
            break;
        }
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);
}