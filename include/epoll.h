#ifndef __EPOLL_H__
#define __EPOLL_H__


#include <sys/epoll.h>
#include <functional>

using std::function;

#define MAXEVENTS 50

class Epoll {
    public:
        Epoll(int maxevents);

        int epollAdd(int fd, int event);
        int epollMod(int fd);
        int epollDel(int fd);

        void epollWait();
	
//	void func(int fd);
        

    public:
        int epoll_fd;
        struct epoll_event *events;
        int nfds;
        int max_events;
//	function<void(int)> handler;
};



#endif
