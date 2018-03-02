
#include "Timer.h"

std::atomic<int64_t> Timer::s_numCreated_(0);

void Timer::restart(Timestamp now) {
    if(isRepeat_) {
        expiration_ = addTime(now, interval_);
    }else {
        expiration_ = Timestamp::invalid();
         //将过期时间设为0，即在之后的时间里，该Timer永远处于过期状态
    }
}