#ifndef SMUDUO_NET_TIMER_H
#define SMUDUO_NET_TIMER_H

#include "../Base/Common.h"
#include "Callback.h"
#include <atomic> //原子操作，C++11
#include <stdint.h>

using namespace smuduo;

class Timer : smuduo::noncopyable {
public:
    Timer(const TimerCallback& cb, Timestamp when, double interval)
    : callback_(cb),
      expiration_(when),
      interval_(interval),
      isRepeat_(interval_ > 0.0),
      sequence_(++s_numCreated_ ) // Since C++11 原子操作
    { }


    void run() const {
        callback_();
    }

    //Timer基本信息获取 Family
    Timestamp expiration() const { return expiration_; }
    double interval() const {return interval_; }
    int64_t sequence() const {return sequence_; }
    bool isRepeat() const { return isRepeat_; }

    void restart(Timestamp now);

private:
    const TimerCallback callback_;      //定时器回调函数
    Timestamp expiration_;              //下一次的超时时刻
    const double interval_;             //超时时间间隔，如果是一次性定时器，该值为0
    const bool isRepeat_;               //是否重复
    const int64_t sequence_;            //定时器序号

    static std::atomic<int64_t> s_numCreated_; //定时器计数，当前已创建的定时器数量


};


#endif











/*
2018-2-8 19:09
此时此刻，我坐在奶奶的房间里，她安安静静地躺在床上
眼神里满是浑浊的光————她已卧床将近一月，身体极度虚弱。
我在想，现在正在写的这个叫做Timer的类，可真该死。

奶奶可能永远也无法了解我现在正在做的事情。
在她看来，我坐在一个会发光的机器前，噼里啪啦手指在乱按
屏幕里出现的也是一些她无法看懂的奇怪字符。

不知道该说些什么，心情极度复杂。


*/