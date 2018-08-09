#ifndef __SOCKETIO_H__
#define __SOCKETIO_H__

#include <sys/types.h>

#define LISTENQ 10
#define RIO_BUFSIZE 8192

typedef struct {
	int rio_fd;
	int rio_cnt;
	char *rio_bufptr;
	char rio_buf[RIO_BUFSIZE];
} rio_t;

typedef struct sockaddr SA;


ssize_t rio_readn(int fd, char *usrbuf, size_t n);

ssize_t rio_writen(int fd, char *usrbuf, size_t n);

void rio_readinitb(rio_t *rp, int fd);

ssize_t rio_readlineb(rio_t *rp, char *usrbuf, size_t maxlen);

ssize_t rio_readnb(rio_t *rp, char *usrbuf, size_t n);

int open_clientfd(char *hostname, char *port);

int open_listenfd(char *port);

int make_socket_non_blocking(int fd);

int TCP_listen(int port);

#endif
