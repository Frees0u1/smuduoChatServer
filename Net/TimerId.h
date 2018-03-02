#ifndef SMUDUO_NET_TIMERID_H
#define SMUDUO_NET_TIMERID_H
#include "../Base/Common.h"
#include "Timer.h"

class TimerQueue;
class Timer;

// TimerId主要用于取消Timer
class TimerId : public smuduo::copyable {
public:
    friend class TimerQueue;
    TimerId(Timer* timer = NULL, int64_t seq = 0)
    : timer_(timer),
      sequence_(seq)
    {
    }

private:
    Timer* timer_;
    int64_t sequence_; //时钟序号
};




#endif