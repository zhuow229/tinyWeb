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

// һ������Ϊ rio_t �Ķ�������
typedef struct {
    int rio_fd;                 // ��û�����ƥ����ļ�������
    int rio_cnt;                // ��������δ�����ֽ���
    char *rio_bufptr;           // ָ�򻺳����е�һ��δ���ֽ�
    char rio_buf[RIO_BUFSIZE];  // ������
} rio_t;

// RIO��
ssize_t rio_readn(int fd, void *usrbuf, size_t n);
ssize_t rio_writen(int fd, void *usrbuf, size_t n);
void rio_readinitb(rio_t *rp, int fd);
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n);
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);

// ��װ����
int Fork();

#endif