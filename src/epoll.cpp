#include "../include/epoll.h"
#include <iostream>
#include <stdlib.h>
#include <signal.h>

using namespace std;


Epoll::Epoll(int maxevents) {
    max_events = maxevents;
    struct epoll_event *ep = (struct epoll_event *)malloc(sizeof(struct epoll_event) * max_events);
    if (ep == NULL) {
        cout << "Epoll: malloc!" << endl;
    }
    events = ep;
    epoll_fd = epoll_create(1024);
    if (epoll_fd <= 0) {
        cout << "Epoll: epoll_create!" << endl;
    }
    struct sigaction act;
        act.sa_handler = SIG_IGN;
        if (sigaction(SIGPIPE, &act, NULL) != 0) {
            cout << "sigaction: " << endl;
            exit(1);
        }
}


int Epoll::epollAdd(int fd, int event) {
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = event;
    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
    if (ret < 0) {
        cout << "Epoll: epollAdd!" << endl;
        return -1;
    }
    else
        return 0;
}


int Epoll::epollMod(int fd) {
    struct epoll_event ev = { 0, {0} };
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
    if (ret < 0) {
        cout << "Epoll: epollMod!" << endl;
        return -1;
    }
    else
        return 0;
    
}


int Epoll::epollDel(int fd) {
    struct epoll_event ev = { 0, {0} }; 
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
    if (ret < 0) {
        cout << "Epoll: epollDel!" << endl;
        return -1;
    }
    else
        return 0;
}


void Epoll::epollWait() {

    nfds = epoll_wait(epoll_fd, events, max_events, -1);
    if (nfds < 0) {
        cout << "Epoll: epollWait!" << endl;
        return;
    }
}















