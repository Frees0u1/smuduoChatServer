/*
Poller Class 是IO multiplexing的封装，现在初步实现为一个具体类，后续要改成抽象类，以同时支持poll和epoll两种IO复用机制。
Poller是EventLoop的间接成员，只供其owner EventLoop在IO线程中调用，因此无需加锁
其生命期与EventLoop相等。Poller并不拥有Channel，Channel在析构之前必须自己unregoster，以免成为空悬指针
*/
#ifndef SMUDUO_NET_POLLER_H
#define SMUDUO_NET_POLLER_H

//#include "Channel.h"
#include "EventLoop.h"
#include "../Base/Timestamp.h"
#include <map>
#include <vector>
#include "../Base/Common.h"

struct pollfd;

class Channel;

/// IO multiplexing with poll(2)
//
// this class doesn't own the Channel object


class Poller : smuduo::noncopyable {
public:
    typedef std::vector<Channel*> ChannelList;

    Poller(EventLoop* loop);
    ~Poller();

    //Polls the IO events
    // Must be called in the loop thread
    Timestamp poll(int timeoutMs, ChannelList* activeChannels);

    // Changes the interested IO events
    // Must be called in loop thread
    void updateChannel(Channel* channel);
    void assertInLoopThread() {ownerLoop_ -> assertInLoopThread(); }

    // Remove the channel, when it destructs
    // Must be called in the loop thread
    void removeChannel(Channel* channel);

private:
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
    typedef std::vector<struct pollfd> PollFdList;
    typedef std::map<int, Channel*> ChannelMap;
    EventLoop *ownerLoop_;
    PollFdList pollfds_;
    ChannelMap channels_;
};

#endif