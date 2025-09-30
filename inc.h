#ifndef __INC_H__
#define __INC_H__

#define _POSIX_C_SOURCE 200112L
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>

#if defined(_WIN32)
#  define strcasecmp _stricmp
#  define strncasecmp _strnicmp
#else
#  include <strings.h>
#endif

#define LISTENQ 128

extern char **environ;

#define RIO_BUFSIZE 8192

// 一个类型为 rio_t 的读缓冲区
typedef struct {
    int rio_fd;                 // 与该缓冲区匹配的文件描述符
    int rio_cnt;                // 缓冲区中未读的字节数
    char *rio_bufptr;           // 指向缓冲区中第一个未读字节
    char rio_buf[RIO_BUFSIZE];  // 缓冲区
} rio_t;

// RIO包
ssize_t rio_readn(int fd, void *usrbuf, size_t n);
ssize_t rio_writen(int fd, void *usrbuf, size_t n);
void rio_readinitb(rio_t *rp, int fd);
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n);
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);

// 包装函数
int Fork();

#endif