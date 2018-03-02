#include "../EventLoop.h"
#include <functional>

#include <cstdio>

//要编译包含EventLoop的文件，以下依赖必须共同加入编译：
// ../EventLoop.cpp
// ../../Base/CurrentThread.cpp
// ../../Base/Timestamp.cpp
// ../Timer.cpp
// ../TimerQueue.cpp
// ../Channel.cpp
// ../Poller.cpp



using namespace std;

int cnt = 0;
EventLoop *g_loop;

void printTid() {
    printf("pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
    printf("now %s\n", Timestamp::now().toFormattedString().c_str());
}

void print(const char* msg) {
    printf("msg %s %s\n", Timestamp::now().toFormattedString().c_str(), msg);
    if(++cnt == 30) {
        g_loop->quit();
    }
}

int main() {
    printTid();
    EventLoop loop;
    g_loop = &loop;
    printf("main()\n");


   /* loop.runEvery(1, ::bind(print, "Every 1 sec"));
    loop.runAfter(2, ::bind(print, "after 2 sec"));*/

     loop.runEvery(2, ::bind(print, "every 2"));

    loop.runAfter(1, ::bind(print, "once 1"));
    loop.runAfter(1.5, ::bind(print, "once 1.5"));
    loop.runAfter(2.5, ::bind(print, "once 2.5"));
    loop.runAfter(3.5, ::bind(print, "once 3.5"));
    
   
    loop.runEvery(3, ::bind(print, "every 3"));

    loop.loop();
    printf("main loop exits");
    sleep(1);


    return 0;
}