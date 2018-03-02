#include "EventLoop.h"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <thread>
#include <poll.h>

void ThreadFunc() {
    printf("threadFunc(): pid = %d, tid = %d\n",
            getpid(), CurrentThread::tid());
    EventLoop loop;
    loop.loop();
}

int main(){
    printf("main(): pid = %d, tid = %d\n",
           getpid(), CurrentThread::tid());
    EventLoop loop;
    
    std::thread thr(ThreadFunc);
    loop.loop();

    thr.join();

    return 0;
}