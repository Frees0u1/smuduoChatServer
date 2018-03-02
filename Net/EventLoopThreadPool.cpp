#include "EventLoopThreadPool.h"
//#include "EventLoop.h"
#include <functional>


using namespace smuduo;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseloop) 
    : baseloop_(baseloop),
      started_(false),
      numThreads_(0),
      next_(0)
{

}

EventLoopThreadPool::~EventLoopThreadPool() {
    // DO NOT delete loop, it's all stack variable
}

void EventLoopThreadPool::start() {
    assert(!started_);
    baseloop_->assertInLoopThread();

    started_ = true;

    for(int i = 0; i < numThreads_; ++i) {
        threads_.push_back(std::make_shared<EventLoopThread>());
        loops_.push_back(threads_.back()->startLoop());
    }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    baseloop_->assertInLoopThread();
    EventLoop* loop = baseloop_;

    if(!loops_.empty()) {
        loop = loops_[next_];
        next_ = (next_ + 1) % loops_.size();
    }

    return loop; //顺次执行而已

    // loops 为空 总是返回baseloop
}