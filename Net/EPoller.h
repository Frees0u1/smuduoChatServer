#ifndef SMUDUO_NET_EPOLLER_H
#define SMUDUO_NET_EPOLLER_H

#include <vector>
#include <map>
#include "../Base/Timestamp.h"
#include "EventLoop.h"

struct epoll_event;

class Channel;

class EPoller : smuduo::noncopyable {
public:
    typedef std::vector<Channel*> ChannelList; //ChannelList在EventLoop中有了定义

    EPoller(EventLoop*  loop);
    ~EPoller();

    // Polls the IO events,
    // must be called in the loop thread
    Timestamp poll(int timeoutMs, ChannelList* activeChannels);

    // Changes  the interested IO events
    // Must be called in the loop thread
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

    void assertInLoopThread() {
        ownerLoop_ -> assertInLoopThread();
    }

private:
    static const int kInitEvnetListSize = 16;
    
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

    void update(int operation, Channel* channel);

    typedef std::vector<struct epoll_event> EventList;
    typedef std::map<int, Channel*> ChannelMap; //sockfd - Channel

    EventLoop* ownerLoop_;
    int epollfd_;
    EventList events_;
    ChannelMap channels_;
};






#endif