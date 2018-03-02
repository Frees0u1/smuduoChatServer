/*
负面测试： 程序将会异常终止
在主线程中创建了EventLoop对象，却试图在另一个线程中调用其EventLoop::loop()
*/
#include "EventLoop.h"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <thread>

EventLoop* g_loop;

void threadFunc(){
    g_loop->loop();
}


int main(){
    EventLoop loop;
    g_loop = &loop;
    std::thread thr(threadFunc);

    thr.join();

    return 0;
}