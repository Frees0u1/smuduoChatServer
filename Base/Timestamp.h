#ifndef SMUDUO_BASE_TIMESTAMP_H
#define SMUDUO_BASE_TIMESTAMP_H

#include <boost/operators.hpp>
#include "../Base/Common.h"


// Time stamp in UTC, in microseconds resolution. 微妙级别的时间戳
// immutable
// It's recommanded to pass it by value, since it's passed in register on x64


class Timestamp : public smuduo::copyable,
                public boost::less_than_comparable<Timestamp>
{
public:
    Timestamp() : microSecondsSinceEpoch_ (0) {

    }
    explicit Timestamp(int64_t microSecondsSinceEpoch);

    void swap(Timestamp &rhs){
        std::swap(microSecondsSinceEpoch_, rhs.microSecondsSinceEpoch_);
    }

    // default copy/op=/dtor are okay
    std::string toString() const;
    std::string toFormattedString(bool showMicroseconds = true) const;
    std::string toShortTZtime(int TZ = 8) const;


    bool valid() const { return microSecondsSinceEpoch_ > 0; }


    // for internal usage
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_;}
    time_t secondsSinceEpoch() const {
        return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    }

    // Get time of now
    static Timestamp now();
    static Timestamp invalid();

    static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t microSecondsSinceEpoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs){
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) {
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

// Gets time difference of two timestamps, result in seconds
//
// @param high, low 
// @return high - low in seconds
inline double timeDifference(Timestamp high, Timestamp low) {
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

// Add @c seconds to given timestamp
//
// @return timestamp+seconds as timestamp
inline Timestamp addTime(Timestamp timestamp, double seconds) {
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}


#endif