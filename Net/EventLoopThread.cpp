#include "EventLoopThread.h"
#include "EventLoop.h"
#include <functional>
#include <assert.h>

EventLoopThread::EventLoopThread() 
    : loop_(nullptr),
      exiting_(false),
      mutex_(),
      cond_()
{
    //thread_暂时不运行程序
 //   printf("LOG_DEBUG: hello from constructor of EventLoopThread\n");
}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    loop_->quit();
    thread_.join();
}



EventLoop* EventLoopThread::startLoop() {
    assert(!thread_.joinable()); //暂未给thread_分配实质的线程工作
    thread_ = std::move(std::thread(std::bind(&EventLoopThread::threadFunc, this)) ); //起一个线程
    
    {
        std::unique_lock<std::mutex> locker(mutex_);
        while(loop_ == NULL) {
            cond_.wait(locker); //Who notice?
        }
    }

    return loop_;
}

void EventLoopThread::threadFunc() { //这个函数的工作就是起一个子线程，然后创一个Loop，在子线程中loop
    EventLoop loop;
    {
        std::unique_lock<std::mutex> locker(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.loop();

    assert(exiting_); //只有退出了loop这个线程才可以消亡
}
