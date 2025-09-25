#include "inc.h"
#include "rio.h"

#define MAXLINE 1024

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);    // ����URI
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg);

int main(int argc, char **argv) {
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    if(argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = open_listenfd(argv[1]);
    while(1) {
        clientlen = sizeof(clientaddr);
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        getnameinfo((struct sockaddr *)&clientaddr, clientlen, hostname, MAXLINE,
                     port, MAXLINE, 0);
        printf("Accepted connection from (%s %s)\n", hostname, port);
        doit(connfd);
        close(connfd);
    }
}

// �򿪲����� ���������� �ĸ�������
int open_listenfd(char *port) {
    struct addrinfo hints, *listp, *p;
    int listenfd, optval = 1;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;
    getaddrinfo(NULL, port, &hints, &listp);

    for(p = listp; p; p = p->ai_next) {
        if((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
            < 0) {
            continue;   // socket failed, try the next
        }

        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
                   (const void *)&optval, sizeof(int));

        if(bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) {
            break;      // bind success
        }
        close(listenfd);    // bind failed, try the next
    }

    freeaddrinfo(listp);
    if(!p) {
        return -1;
    }

    if(listen(listenfd, LISTENQ) < 0) {
        close(listenfd);
        return -1;
    }
    return listenfd;
}

// ����һ�� HTTP ����
void doit(int fd) {
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    // ������
    rio_readinitb(&rio, fd);
    rio_readlineb(&rio, buf, MAXLINE);
    printf("Request headers:\n");
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    if(strcasecmp(method, "GET")) {     // method != "GET"
        clienterror(fd, method, "501", "Not implemented",
                    "tinyWeb does not implement this method");
        return;
    }
    read_requesthdrs(&rio);

    // ���� URI
    is_static = parse_uri(uri, filename, cgiargs);
    if(stat(filename, &sbuf) < 0) {
        clienterror(fd, filename, "404", "Not found",
                    "tinyWeb couldn't read the file");
        return;
    }

    if(is_static) {     // ����̬����
        if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden",
                        "tinyWeb couldn't read the file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size);
    }
    else {      // ����̬����
        if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
            clienterror(fd, filename, "403", "Forbidden",
                        "tinyWeb couldn't run the CGI program");
            return;
        }
        serve_dynamic(fd, filename, cgiargs);
    }
}

// ������
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg) {
    char buf[MAXLINE], body[MAXLINE];

    // build the HTTP response body
    sprintf(body, "<html><title>tinyWeb Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The tinyWeb server</em>\r\n", body);

    // ��� HTTP response
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    rio_writen(fd, buf, sizeof(buf));
    rio_writen(fd, body, sizeof(body));
}

// ��ȡ����ͷ (����������ڵ�����ͷ)
void read_requesthdrs(rio_t *rp) {
    char buf[MAXLINE];

    rio_readlineb(rp, buf, MAXLINE);
    while(strcmp(buf, "\r\n")) {
        rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}

// ���� URI
int parse_uri(char *uri, char *filename, char *cgiargs) {
    char *ptr;

    if(!strstr(uri, "cgi-bin")) {   // ��̬����
        strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        if(uri[strlen(uri - 1)] == '/') {
            strcat(filename, "home.html");
        }
        return 1;
    }
    else {      // ��̬����
        ptr = index(uri, '?');
        if(ptr) {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        } else {
            strcpy(cgiargs, "");
        }
        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
    }
}

char * index(char *dest, const char c) {
    int n = strlen(dest);
    for(int i = 0; i < n; i++) {
        if(dest[i] == c) {
            return &dest[i];
        }
    }
    return NULL;
}

