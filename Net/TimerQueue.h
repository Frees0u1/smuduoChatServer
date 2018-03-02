#ifndef SMUDUO_NET_TIMERQUEUE_H
#define SMUDUO_NET_TIMERQUEUE_H
#include "../Base/Common.h"
#include "../Net/Callback.h"
#include "../Base/Timestamp.h"
#include "../Net/EventLoop.h"
#include "../Net/Channel.h"


/*
* 时间队列的本质：用一个Set存好几个定时器，定时器自己不会自己定时，只是存在set中的一堆数据
* 所以在任何时刻，Set里有只有两类定时器，到期的和未到期的
* 
* 在EventLoop中，loop的每一次循环都会调用poll
*    如果Set中最近的那个时刻到期,则回触发事件返回，Poller启动HandleEvent程序，从而启动TimeQueue的handleRead()程序
*    此时handleRead()则会调取这个时间段内所有已到期的事件，一一顺次处理（根据Timer所设置的回调），并把此时最近需要触发的事件更新（通过timerfd_settime）
*    如果Poll周期内没有事件到期，则重启下一轮循环
* addTimer中，会判断每次新添的Timer是否会改变目前最早的那个timer,主要有以下两种情况：
    1. 目前的TimerQueue为空
    2. 新加的这个Timer到期时间比最早的还要早
    3. 将目前所有已到期的更新后，剩下的需要更新
  以上三种情况均需要
*/



//#include "Timer.h"
//#include "TimerId.h"


#include <set>
#include <vector>



class Timer;
class TimerId;



// A best efforts timer queue
// No guarantee the callback will be on time
//
class TimerQueue : smuduo::copyable {
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    //必须也一定是线程安全的，通常情况下被其他线程所调用
    TimerId addTimer(const TimerCallback &cb, Timestamp when, double interval);

    void cancel(TimerId timerId);

    //for-debug
    //int currentTQsize()const { return  timers_.size(); }


private:
    typedef std::pair<Timestamp, Timer* > Entry;
    struct EntryComp { //自定义的比较器，首先比较时间（大者大），然后比较Timer的unique_ptr指针，小者大
        bool operator()(const Entry& lhs, const Entry& rhs) {
            if(lhs.first == rhs.first) {
                return lhs.second > rhs.second;
            }else {
                return lhs.first < rhs.first;
            }
        }
    };
    
    
    typedef std::set<Entry, EntryComp> TimerSet; //有自定义比较器的Set, 底层是一棵红黑树
    typedef std::pair<Timer*, int64_t> ActiveTimer;
    typedef std::set<ActiveTimer> ActiveTimerSet;

    EventLoop *loop_;           // 所属EventLoop
    const int timerfd_;         // timerfd_create 函数创建
    Channel timerfdChannel_;    // 管理timerfd的专属Channel

    TimerSet timers_; 

    // for cancel
    bool callingExpiredTimers_; // atomic
    ActiveTimerSet activeTimers_;
    ActiveTimerSet cancelingTimers_;


    //底层实现细节函数
    //
    //getExpired
    // 从timers_移除已到期的Timer,并通过vector返回他们，编译器会实施RVO优化
    std::vector<Entry> getExpired(Timestamp now);

    //called when timerfd alarms
    void handleRead();
    //将所有到期的Timer从TimerSet中移除出去
    void reset(const std::vector<Entry> & expired, Timestamp now);

    //给TimerSet添加新的timer, 当然，首先要做的工作就是将Timer转化为Entry
    bool insert(Timer*);
   // bool insert(const std::unique_ptr<Timer>& timer);

   // addTimerInLoop
   void addTimerInLoop(Timer* timer);

    void cancelTimerInLoop(TimerId timerId);
};


#endif