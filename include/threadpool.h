#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <pthread.h>
#include <vector>
#include <functional>
#include "Cache.h"

using std::vector;
using std::function;
using namespace nzj;

class ThreadPool {
    public:
        ThreadPool(int threadnum, function<void(int,Cache&)> handle);
        void addTask(int fd);
        void start();
 //       void stop();
       	static void *pthfunc(void *arg);



    public:
        pthread_t *threads;
	pthread_mutex_t lock;
	pthread_mutex_t lock2;
	pthread_cond_t cond;
        int pthnum;
        int tasknum;
	int curpth;	
        vector<int> taskqueue;
	function<void(int,Cache&)> handler;
	vector<Cache> _cacheVec;	//初始值为pthnum个空的Cache
};







#endif

