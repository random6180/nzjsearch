#include "threadpool.h"
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <algorithm>
using namespace std;


ThreadPool::ThreadPool(int threadnum, function<void(int,Cache&)> handle) {
    pthnum = threadnum;
    tasknum = 0;
    curpth = 0;
    pthread_t *threadtmp = (pthread_t *)malloc(sizeof(pthread_t) * threadnum);
    threads = threadtmp;
    handler = handle;
    
    Cache tmp;
    for (int i = 0; i <= threadnum; i++) {
        _cacheVec.push_back(tmp);
    }

    if (pthread_mutex_init(&lock, NULL) != 0) 
        cout << "ThreadPool: pthread_mutex_init!" << endl;
    if (pthread_cond_init(&cond, NULL) != 0)
        cout << "ThreadPool: pthread_cond_init!" << endl;

    if (pthread_mutex_init(&lock2, NULL) != 0)
        cout << "ThreadPool: pthread_mutex_init!" << endl;
}   


void ThreadPool::addTask(int fd) {
    if (pthread_mutex_lock(&lock) != 0) {
        cout << "addTask: pthread_mutex_lock!" << endl;
        return;
    }

    if (find(taskqueue.begin(), taskqueue.end(), fd) == taskqueue.end()) { 
            taskqueue.push_back(fd);
            pthread_cond_signal(&cond);
            cout << "addTask: tasknum " << taskqueue.size() << endl;
    }
    pthread_mutex_unlock(&lock);
}


void ThreadPool::start() {
    for (int i = 0; i < pthnum; i++) {
        curpth++;
        if (pthread_create(&threads[i], NULL, pthfunc, (void *)this) != 0 ) {
            cout << "ThreadPool: start!" << endl;
        }
    }
}


void *ThreadPool::pthfunc(void *arg) {
    ThreadPool *tmp = (ThreadPool *)arg;
    int cur = tmp->curpth;
    int n = 0;
    while(1) {
        pthread_mutex_lock(&tmp->lock);
        cout << "线程:" << pthread_self() << endl;
        while (tmp->taskqueue.size() == 0)
            pthread_cond_wait(&tmp->cond, &tmp->lock);
       
        if (tmp->taskqueue.size() > 0) {
            int fd = tmp->taskqueue[0];
            tmp->taskqueue.erase(tmp->taskqueue.begin());
//            tmp->tasknum--;
            pthread_mutex_unlock(&tmp->lock);
            cout << "handler: " << fd << endl;
            tmp->handler(fd, tmp->_cacheVec[cur]);
            n++;
        }

        if (n == 2) {
            pthread_mutex_lock(&tmp->lock2);
            cout << "更新缓存:" << endl;
            tmp->_cacheVec[0].updateBy(tmp->_cacheVec[cur]);
            n = 0;
            tmp->_cacheVec[cur] = tmp->_cacheVec[0];
            pthread_mutex_unlock(&tmp->lock2);
        }
            
    }
}
