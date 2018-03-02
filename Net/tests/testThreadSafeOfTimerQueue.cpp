#include "../EventLoop.h"

#include <iostream>
#include <thread>
#include <functional>

using namespace std;

EventLoop* g_loop;
int cnt = 0;
void print(const char *msg) {
   printf("msg ： %s\n", msg);
   if(++cnt == 20) {
       g_loop->quit();
   }
} //空函数

void threadFunc() {
    printf("threadFunc(), pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
    g_loop->runEvery(2, ::bind(print, "ThreadFunc(): Every 2"));
}


int main() {

    printf("Main(), pid = %d, tid = %d\n", getpid(), CurrentThread::tid());

    EventLoop loop;
    g_loop =  &loop;
    ::thread thr(threadFunc);
    g_loop->runEvery(1, ::bind(print, "main(): Every 1"));
     thr.join();
    loop.loop(); 

  //  sleep(10);

  //  loop.quit();

   

    return 0;
}