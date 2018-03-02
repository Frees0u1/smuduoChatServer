#ifndef SMUDUO_NET_EVENTLOOP_H
#define SMUDUO_NET_EVENTLOOP_H

#include "../Base/Common.h"
#include "../Base/CurrentThread.h"
#include "../Base/Mutex.h"

#include <unistd.h>
#include <cstdlib>
#include <vector>
#include <memory>
#include <functional>

#include "../Base/Timestamp.h"
#include "TimerId.h"
#include "Callback.h"

class Channel;
class EPoller;
class TimerQueue;

typedef std::function<void()> Functor;

using namespace smuduo;

class EventLoop : smuduo::noncopyable {
public:
    EventLoop();
    ~EventLoop();
    
    void loop();

    void assertInLoopThread() {
        if(!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread() const {
        return threadId_ == CurrentThread::tid(); 
    }

    EventLoop* getEventLoopOfCurrentThread();
    
    void quit();
    void updateChannel(Channel* channel);

    //2018.2.8 add runInLoop
    void runInLoop(const Functor& cb);
    void queueInLoop(const Functor& cb);    
    
    //internal usage
    void wakeup();
    

  //  bool hasChannel(Channel* chptr);

    //2018.2.9 定时器功能 TimerQueue
    TimerId runAt(const Timestamp &time, const TimerCallback &cb);
    TimerId runAfter(double delay, const TimerCallback &cb);
    TimerId runEvery(double interval, const TimerCallback &cb);
    void cancel(TimerId timerId);

    // 2018.2.23 处理TcpConnection连接关闭的需要
    void removeChannel(Channel* chptr);

    Timestamp pollReturnTime() const { return pollReturnTime_; }

private:
    void abortNotInLoopThread();
    
    typedef std::vector<Channel*> ChannelList;

    bool looping_; /*atomic*/
    bool quit_; /*atomic*/
    std::unique_ptr<EPoller> poller_;
    ChannelList activeChannels_;
    const pid_t threadId_; 

    //2018.2.8 增加一个runInLoop功能
    void handleRead(); // wake up;
    void doPendingFunctors();

    bool callingPendingFunctors_; /*atomic*/
    int wakeupFd_; 
    std::unique_ptr<Channel> wakeupChannel_;
    MutexLock mutex_;
    std::vector<Functor> pendingFunctors_; // GuardedBy mutex_;

    //增加TimeQueue功能,即定时器功能
    std::unique_ptr<TimerQueue> timerQueue_;
    Timestamp pullReturnTime_;

    Timestamp pollReturnTime_;
};

#endif