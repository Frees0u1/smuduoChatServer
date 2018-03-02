#include "EventLoop.h"
#include "Channel.h"
#include "EPoller.h"
#include <string.h>
#include <assert.h>
#include <poll.h>
#include <string>
#include <cstdio>
#include <signal.h>
#include <sys/eventfd.h>

#include "TimerQueue.h"


namespace {
__thread EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

static int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0) {
        printf("LOG_SYSERR: Failed in eventfd");
        exit(-1);
    }
    return evtfd;
}


#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe
{
 public:
  IgnoreSigPipe()
  {
    ::signal(SIGPIPE, SIG_IGN);
    // LOG_TRACE << "Ignore SIGPIPE";
  }
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;
}


void fatal_handler (const char* msg){
    printf("%s\n", msg);
    exit(-1);
}

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      callingPendingFunctors_(false),
      threadId_(CurrentThread::tid()), 
      poller_(std::make_unique<EPoller>(this) ),
      wakeupFd_(createEventfd()),
      wakeupChannel_(std::make_unique<Channel>(this, wakeupFd_)),
      timerQueue_(std::make_unique<TimerQueue>(this))
{
    printf("LOG TRACE: EventLoop created %p in thread %d wakeupFd = %d\n", this, threadId_, wakeupFd_);
    if(t_loopInThisThread) {
        fatal_handler("Another EventLoop already existed!");
    }
    else{
        t_loopInThisThread = this;
    }
    wakeupChannel_->setReadCallback(
        std::bind(&EventLoop::handleRead, this)
    );
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    // LOG DEBUG msg
  //  wakeupChannel_->disableAll();
    //wakeupChannel_->remove();
    assert(!looping_);
    ::close(wakeupFd_);
    t_loopInThisThread = NULL;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread() {
    return t_loopInThisThread;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    while(!quit_) {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for(auto it = activeChannels_.begin(); it != activeChannels_.end(); ++it) {
            (*it)->handleEvent(pollReturnTime_);
        }
        doPendingFunctors();
    }
    
    
    //printf("LOG_TRACE: EventLoop %p stop looping\n", this);
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    
    if(!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::updateChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}


void EventLoop::runInLoop(const Functor& cb) {
    if(isInLoopThread()) {
        cb();
    } else {
        queueInLoop(cb);
    }
}


void EventLoop::queueInLoop(const Functor& cb) {
    {
    MutexLockGuard lock(mutex_);
    pendingFunctors_.push_back(cb);
    }
//    printf("QueueInLoop: wakeupFd_ = %d\n", wakeupFd_);
    if(!isInLoopThread() || callingPendingFunctors_ ) {
        wakeup();
    }   
}


void EventLoop::removeChannel(Channel* chPtr){
    assert(chPtr->ownerLoop() == this);
    assertInLoopThread();
    poller_->removeChannel(chPtr);
}


void EventLoop::abortNotInLoopThread() {
    printf("LOG_FATAL: EventLoop::abortNotInLoopThread - EventLoop %p was created in threadId_ = %d, current thread = %d\n"
    , this, threadId_, CurrentThread::tid());
    exit(-1);
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof one);
    if(n != sizeof one) {
        printf("EventLoop::wakeup writes %ld bytes instead of 8\n", n);
    }
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_  = true;

    {
    MutexLockGuard lock(mutex_);
    functors.swap(pendingFunctors_);
    }

    for(size_t i = 0; i < functors.size(); ++i) {
        functors[i]();
    }
    callingPendingFunctors_ = false;
}


void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof one);
    if(n != sizeof one ){
        printf("LOG_ERROR: EventLoop::handleRead() reads %ld bytes instead of 8", n);
    }
}


//定时器功能
TimerId EventLoop::runAt(const Timestamp &time, const TimerCallback &cb) {
    return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb) {
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback &cb) {
    Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(cb, time, interval);
}

void EventLoop::cancel(TimerId timerId) {
    return timerQueue_ -> cancel(timerId);
}
