#include "EventLoop.h"
#include "Poller.h"
#include "Channel.h"
#include <cstdio>

#include <sys/timerfd.h>

EventLoop* g_loop;

void timeout() {
    printf("Timeout!\n");
    g_loop->quit();
}

int main() {
    EventLoop loop;
    g_loop = &loop;

    int timerfd1 = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    int timerfd2 = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    int timerfd3 = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    
    Channel channel1(&loop, timerfd1);
    channel1.setReadCallback(timeout);
    channel1.enableReading();

     Channel channel2(&loop, timerfd2);
    channel2.setReadCallback(timeout);
    channel2.enableReading();

     Channel channel3(&loop, timerfd3);
    channel3.setReadCallback(timeout);
    channel3.enableReading();

    struct itimerspec howlong;
    bzero(&howlong, sizeof howlong);
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd1, 0, &howlong, NULL);
    ::timerfd_settime(timerfd2, 0, &howlong, NULL);
    ::timerfd_settime(timerfd3, 0, &howlong, NULL);


    loop.loop();

    ::close(timerfd1);
    ::close(timerfd2);
    ::close(timerfd3);
}