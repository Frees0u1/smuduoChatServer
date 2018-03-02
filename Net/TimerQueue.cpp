#include "TimerQueue.h"
#include <sys/timerfd.h>


int createTimerfd() {
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if(timerfd < 0) {
        printf("LOG_SYSFATAL: Failed in timerfd_create");
    }
    return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when) {
    int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
    if(microseconds < 100) {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t> (
        microseconds / Timestamp::kMicroSecondsPerSecond
    );

    ts.tv_nsec =  static_cast<time_t> (
        microseconds % Timestamp::kMicroSecondsPerSecond * 1000
    );
    return ts;
}

void resetTimerfd(int timerfd, Timestamp expiration) {
    //wake_up loop by timerfd_settime();
    struct itimerspec newVal;
    struct itimerspec oldVal;
    bzero(&newVal, sizeof newVal);
    bzero(&oldVal, sizeof oldVal);

    newVal.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newVal, &oldVal);
    if(ret) {
        printf("LOG_SYSERR: something bad happened in timerfd_settime()");
    }
}

void readTimerfd(int timerfd, Timestamp now) {
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    //printf("LOG_TRACE: TimerQueue::handleRead() reads %ld bytes at %s\n", howmany, now.toFormattedString().c_str());
    if(n != sizeof howmany) {
        printf("LOG_ERROR: TimerQueue::handleRead() reads not 8 bytes");
    }
}


TimerQueue::TimerQueue(EventLoop* loop)
:   loop_(loop),
    timerfd_ (createTimerfd()),
    timerfdChannel_(loop, timerfd_),
    timers_(), /*空set*/
    callingExpiredTimers_(false)
{
    timerfdChannel_.setReadCallback(
        std::bind(&TimerQueue::handleRead, this)
    );
    timerfdChannel_.enableReading();
}


TimerQueue::~TimerQueue() {
   // timerfdChannel_.disableAll();
    //timerfdChannel_.remove();
    // do not remove channel, since we're in EventLoop::dtor();
    ::close(timerfd_);
   
    for(auto& e : timers_){
        delete e.second;
    }
}

TimerId TimerQueue::addTimer(const TimerCallback &cb,
                             Timestamp when,
                             double interval) {
    Timer* timer = new Timer(cb, when, interval);
    loop_->runInLoop(
        std::bind(&TimerQueue::addTimerInLoop, this, timer)
    );

    return TimerId(timer, timer->sequence());
}

void TimerQueue::addTimerInLoop(Timer* timer) {
    loop_->assertInLoopThread();
    bool earliestChanged = insert(timer);

    if(earliestChanged) {
        resetTimerfd(timerfd_, timer->expiration());
    }
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
    assert(timers_.size() == activeTimers_.size());
    std::vector<Entry> expired;
    Entry sentry = std::make_pair(now, nullptr);
    //sentry是哨兵节点，是时间线为now的最大者，且自身不存在
    auto iter = timers_.lower_bound(sentry); //iter所指向的位置是第一个暂时还没有到期的Timer
    assert(iter == timers_.end() || now < iter->first);
    std::copy(timers_.begin(), iter, back_inserter(expired));
    timers_.erase(timers_.begin(), iter);

    for(auto& entry: expired){ //所有到期的Timer都要删除
        ActiveTimer  timer(entry.second, entry.second->sequence());
        size_t n = activeTimers_.erase(timer);
        assert(n == 1); (void)n;
    }
    assert(timers_.size() == activeTimers_.size());
    return expired;
}


void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_, now);
    
    std::vector<Entry> expired = getExpired(now);

    callingExpiredTimers_ = true;
    cancelingTimers_.clear();
    for(auto iter = expired.begin(); iter != expired.end(); ++iter) {
        iter->second-> run(); //different call it's callback_ own
    }
    callingExpiredTimers_ = false;

    reset(expired, now);
    //printf("current TimerSet size: %u\n", timers_.size());
}

bool TimerQueue::insert(Timer* timer){
    
    assert(timers_.size() == activeTimers_.size());
    loop_->assertInLoopThread();
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    auto iter = timers_.begin();
    if(iter == timers_.end() || when < iter->first) {
        earliestChanged = true;
    }
    {
        std::pair<TimerSet::iterator, bool> result = 
        timers_.insert(std::make_pair(when, timer));
        assert(result.second); //确保插入成功
        (void) result;
    }

    {
        std::pair<ActiveTimerSet::iterator, bool> result
        = activeTimers_.insert(std::make_pair(timer, timer->sequence()));
        assert(result.second); (void) result;
    }
    assert(timers_.size() == activeTimers_.size());
    
    return earliestChanged;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now) {
    Timestamp nextExpire;

    for(auto iter = expired.begin(); iter != expired.end(); ++iter) {
        ActiveTimer timer(iter->second, iter->second->sequence());

        if(iter->second->isRepeat() 
          && cancelingTimers_.find(timer) == cancelingTimers_.end()){
            iter->second->restart(now);
            insert(iter->second); 
        } else {
            // if use raw poiner to manager timer, it must delete here
            // unique_ptr, it wound be destory timer in handleRead(), and it wound be a diaster
            delete iter->second;
        } 
    }

    if(!timers_.empty()) {
        nextExpire = timers_.begin()->second->expiration();
    }
    if(nextExpire.valid()) {
        resetTimerfd(timerfd_, nextExpire);
    }
}


void TimerQueue::cancel(TimerId timerId) {
    loop_->runInLoop(
        std::bind(&TimerQueue::cancelTimerInLoop, this, timerId)
    );
}

void TimerQueue::cancelTimerInLoop(TimerId timerId) {
    loop_->assertInLoopThread();
    //printf("timers_.size() = %u, activeTimers_.size() = %u\n",
    //        timers_.size(), activeTimers_.size());
    assert(timers_.size() == activeTimers_.size());

    // 因为TimerQueue是TimerId的friend class, 所以下面这一步没有问题
    ActiveTimer timer(timerId.timer_, timerId.sequence_);
    auto iter = activeTimers_.find(timer);

    if(iter != activeTimers_.end()) {
        size_t n = timers_.erase(Entry(iter->first->expiration(), iter->first));
        assert(n == 1); (void) n;
        delete iter->first; // FIXME: 手动使用的Delete
        activeTimers_.erase(iter);
    }
    else if(callingExpiredTimers_) { //iteri为空
        cancelingTimers_.insert(timer);
    }
    assert(timers_.size() == activeTimers_.size());
}