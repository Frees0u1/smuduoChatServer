#ifndef SMUDUO_NET_EVENTLOOPTHREADPOOL_H
#define SMUDUO_NET_EVENTLOOPTHREADPOOL_H

#include "../Base/Common.h"
#include "EventLoop.h"
#include "EventLoopThread.h"

#include <vector>
#include <memory>

class EventLoopThreadPool : smuduo::noncopyable {
public:
    EventLoopThreadPool(EventLoop* baseloop);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) {
        numThreads_ = numThreads;
    }
    void start();
    EventLoop* getNextLoop();


private:
    EventLoop*  baseloop_;
    bool started_;
    int numThreads_;
    int next_; // always in loop  thread
    std::vector<std::shared_ptr<EventLoopThread> > threads_; //线程库
    std::vector<EventLoop* > loops_;
};

#endif