#include "../include/tcpserver.h"
#include "../include/epoll.h"
#include "../include/socketio.h"
#include "../include/tiny.h"
#include "../include/threadpool.h"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <string>

using std::cout;
using std::endl;
using std::string;


Tcpserver::Tcpserver(int _port, int pthnum, int epnum) {
    port = _port;
    pthread_num = pthnum;
    event_num = epnum;
}


void Tcpserver::Serve() {
    listen_fd = open_listenfd(const_cast<char *>(std::to_string(port).c_str()));
    cout << "listen_fd: " << listen_fd << endl;

    socklen_t clientlen;
    struct sockaddr_in clientaddr;

    ThreadPool pool(pthread_num, doit);
    pool.start();

    Epoll ep(event_num);
    ep.epollAdd(listen_fd, EPOLLIN | EPOLLET);

    while (1) {
        ep.epollWait();
        if (ep.nfds < 0) {
            cout << "epollWait: " << ep.nfds << endl;
            continue;
        }

        for (int i = 0; i < ep.nfds; i++) {
            if (ep.events[i].data.fd == listen_fd) {
                clientlen = sizeof(clientaddr);
                int connfd;
                if ((connfd = accept(listen_fd, (struct sockaddr*)&clientaddr, &clientlen)) < 0) {
                    cout << "accept!" << endl;
                }
                cout << "监听: " << connfd << endl;
                ep.epollAdd(connfd, EPOLLIN|EPOLLET|EPOLLONESHOT);
            }
            else if (ep.events[i].events & EPOLLIN) {
                int cfd = ep.events[i].data.fd;
                cout << "读取:" << cfd << endl;
                pool.addTask(cfd);
            }
        }
    }
}
