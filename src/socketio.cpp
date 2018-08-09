#include "../include/socketio.h"
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using std::cout;
using std::endl;

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n);

ssize_t rio_readn(int fd, char *usrbuf, size_t n) {
	size_t nleft = n;
	ssize_t nread;
	char *bufp = usrbuf;

	while (nleft > 0) {
		if ((nread = read(fd, bufp, nleft)) < 0) {
			if (errno == EINTR)
				nread = 0;
			else
				return -1;
		}
		else if (nread == 0)
			break;
		nleft -= nread;
		bufp += nread;
	}
	return (n -  nleft);
}


ssize_t rio_writen(int fd, char *usrbuf, size_t n) {
        size_t nleft = n;
        ssize_t nwritten;
        char *bufp = usrbuf;

        while (nleft > 0) {
                if ((nwritten = write(fd, bufp, nleft)) <= 0) {
                        if (errno == EINTR)
                                nwritten = 0;
                        else
                                return -1;
                }
                nleft -= nwritten;
                bufp += nwritten;
        }
        return n;
}


void rio_readinitb(rio_t *rp, int fd) {
        rp->rio_fd = fd;
        rp->rio_cnt = 0;
        rp->rio_bufptr = rp->rio_buf;
}


static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n) {
        int cnt;

        while (rp->rio_cnt <= 0) {
                rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
                if (rp->rio_cnt < 0) {
                        if (errno != EINTR)
                                return -1;
                }
                else if (rp->rio_cnt == 0)
                        return 0;
                else
                        rp->rio_bufptr = rp->rio_buf;
        }

        cnt = n;
        if (rp->rio_cnt < (int)n)
                cnt = rp->rio_cnt;
        memcpy(usrbuf, rp->rio_bufptr, cnt);
        rp->rio_bufptr += cnt;
        rp->rio_cnt -= cnt;

        return cnt;
}


ssize_t rio_readlineb(rio_t *rp, char *usrbuf, size_t maxlen) {
        int n, rc;
        char c, *bufp = usrbuf;

        for (n = 1; n < (int)maxlen; n++) {
                if ((rc = rio_read(rp, &c, 1)) == 1) {
                        *bufp++ = c;
                        if (c == '\n') {
                                n++;
                                break;
                        }
                }
                else if (rc == 0) {
                        if (n == 1)
                                return 0;
                        else
                                break;
                }
                else
                        return -1;
        }
        *bufp = 0;
        return n-1;
}


ssize_t rio_readnb(rio_t *rp, char *usrbuf, size_t n) {
        size_t nleft = n;
        ssize_t nread;
        char *bufp = usrbuf;

        while (nleft > 0) {
                if ((nread = rio_read(rp, bufp, nleft)) < 0)
                        return -1;
                else if (nread == 0)
                        break;
                nleft -= nread;
                bufp += nread;
        }

        return (n - nleft);
}


int open_clientfd(char *hostname, char *port) {
        int clientfd;
        struct addrinfo hints, *listp, *p;

        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_NUMERICSERV;
        hints.ai_flags |= AI_ADDRCONFIG;
        getaddrinfo(hostname, port, &hints, &listp);

        for (p = listp; p; p = p->ai_next) {
                if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
                        continue;

                if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
                        break;
                close(clientfd);
        }

        freeaddrinfo(listp);
        if (!p)
                return -1;
        else
                return clientfd;
}


int open_listenfd(char *port) {
        struct addrinfo hints, *listp, *p;
        int listenfd, optval = 1;

        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
        hints.ai_flags |= AI_NUMERICSERV;
        getaddrinfo(NULL, port, &hints, &listp);

        for (p = listp; p; p = p->ai_next) {
                if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
                        continue;

                setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));

                if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
                        break;

                close(listenfd);
        }

        freeaddrinfo(listp);
        if (!p)
                return -1;

        if (listen(listenfd, LISTENQ) < 0) {
                close(listenfd);
                return -1;
        }
        return listenfd;
}


int make_socket_non_blocking(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    if (flag == -1)
        return -1;
    flag |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flag) == -1)
        return -1;
    return 0;
}


int TCP_listen(int port) {
    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    if (listenfd == -1) {
        cout << "TCP_listen: socket!" << endl;
        return -1;
    }

    int flag = fcntl(listenfd, F_GETFL, 0);
    if (flag == -1) {
        cout << "TCP_listen: fcntl!" << endl;
        return -1;
    }

    flag |= O_NONBLOCK;
    flag = fcntl(listenfd, F_SETFL, flag);
    if (flag == -1) {
        cout << "TCP_listen: fcntl!" << endl; 
    }

//    int on = 1;
//    int ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
//    if (ret == -1) {
//      cout << "TCP_listen: setsockopt!" << endl;
//        return -1;
//    }

    sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        cout << "TCP_listen: bind!" << endl;
        return -1;
    }
    listen(listenfd, 100);
    return listenfd;
} 
































