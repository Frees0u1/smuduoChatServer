#ifndef SMUDUO_NET_EVENTTHREADLOOP_H
#define SMUDUO_NET_EVENTTHREADLOOP_H 


#include "EventLoop.h"
#include "../Base/Common.h"


#include <mutex>
#include <thread>
#include <condition_variable>

class EventLoop;

class EventLoopThread : smuduo::noncopyable {
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* startLoop();

private:
    void threadFunc();

    EventLoop *loop_;
    bool exiting_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
};


#endif