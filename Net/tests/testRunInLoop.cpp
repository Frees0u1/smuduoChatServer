#include "../EventLoop.h"
#include <cstdio>

EventLoop* g_loop;
int g_flag = 0;


int main() {
    printf("main: pid = %d, flag = %d\n", getpid(), g_flag);
    EventLoop loop;
    g_loop = &loop;
}