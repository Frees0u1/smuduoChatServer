#include "../EventLoop.h"
#include "../TimerQueue.h"

#include "../TimerId.h"

EventLoop* g_loop;
TimerId toCancel;

void cancelSelf(){
    printf("cancelSelf()\n");
    g_loop->cancel(toCancel);
}

int main() {
    EventLoop loop;
    g_loop = &loop;

    toCancel = loop.runEvery(2, cancelSelf);
    loop.loop();

    return 0;
}   