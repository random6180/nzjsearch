#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__


class Tcpserver {
    public:
        Tcpserver(int _port, int pthnum, int epnum);
        void Serve();

    public:
        int port;
	int listen_fd;
	int pthread_num;
	int event_num;
};


#endif
