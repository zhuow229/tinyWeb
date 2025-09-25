/* RIO ��ȫ��Ϊ Robust IO ��������
 * ���к����Ƕ� Linux ���� I/O �����ķ�װ��
 * ʹ����ӽ�׳����Ч���������������̡�
 * ������˵�������Զ������д�еĲ���ֵ�����
 * �������������Ӧ���о������֣�
 * ��˱�д�������ʱ�����Ǿ����õ�����
 */
/* CSAPP ��ʵ�ֵ� RIO �ṩ�����಻ͬ�ĺ����� */
/* **�޻���� I/O ����**
 * ��Ӧ�ü��Ļ��壬�Զ��������ݶ�д������
 * �ʹ������д��������������Ϊ���á�
 *
 * **�л���� I/O ����**
 * ���ı���ȡ�ı��кͶ����ƣ����ᱻ������Ӧ�ü��������У�
 * �û����������԰�ȫ�ġ���ͬһ���������Ͽ��Ա�����ص��á�
 */

/************************************************************/

#include "inc.h"

#define RIO_BUFSIZE 8192

// �޻������� I/O ����
/* ֱ�����ڴ���ļ�֮�䴫�����ݣ�û��Ӧ�ü����塣
 * �����������ݶ�д������ʹ������д���������ݡ�
 */
ssize_t rio_readn(int fd, void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while(nleft > 0) {
        if((nread = read(fd, bufp, nleft)) < 0) {
            if(errno == EINTR) {    // ���źŴ������ķ����ж�
                nread = 0;          // ���µ��� read()
            } else {
                return -1;          // read() ����
            }
        }
        else if(nread == 0) {
            break;                  // EOF
        }
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);             // ���ض������ֽ���
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
    return n;   // ������ʱһ����ȫ��д��
}


// ����������뺯��
/* ���ļ��ж�ȡ�ı��кͶ��������ݣ�
 * �ļ������ݻ�����Ӧ�ü��������ڡ�
 * �̰߳�ȫ����ͬһ���������Ͽ��Ա�����ص���
 */
// һ������Ϊ rio_t �Ķ�������
typedef struct {
    int rio_fd;                 // ��û�����ƥ����ļ�������
    int rio_cnt;                // ��������δ�����ֽ���
    char *rio_bufptr;           // ָ�򻺳����е�һ��δ���ֽ�
    char rio_buf[RIO_BUFSIZE];  // ������
} rio_t;

// ��ʼ��������, �������� fd ���ַ rp ���Ķ���������ϵ����
void rio_readinitb(rio_t *rp, int fd) {
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

// rio_read ������ RIO ��������ĺ��ģ�Linux read �����Ĵ�����汾
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n) {
    int cnt;
    // �Ƚ������������ݶ���������, �ٴӻ��������� usrbuf
    while(rp->rio_cnt <= 0) {       // ��������ʱ���
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        if(rp->rio_cnt < 0) {
            if(errno != EINTR) {    // ���źŴ������ķ��ش�������µ���read
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

/* rio_readlineb �Ӷ�������(rp��)����һ���ı���(�������з�)��usrbuf��
 * ���� NULL �ַ���������ı���, ���� maxlen-1 ���ֽ�(��NULL��һ���ֽ�)��
 * �����������ʱ���Զ�����read��������������
 */
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) {
    int n, rc;
    char c, *bufp = usrbuf;

    for(n = 1; n < maxlen; n++) {
        if((rc = rio_read(rp, &c, 1)) == 1) {
            *bufp++ = c;    // �ȼ��� *bufp = c; bufp++;
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