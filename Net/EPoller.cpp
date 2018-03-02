#include "EPoller.h"
#include "Channel.h"

#include <type_traits> //for static_assert

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>

// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same
static_assert(EPOLLIN == POLLIN, "");
static_assert(EPOLLPRI == POLLPRI, "");
static_assert(EPOLLOUT == POLLOUT, "");
static_assert(EPOLLRDHUP == POLLRDHUP, "");
static_assert(EPOLLERR == POLLERR, "");
static_assert(EPOLLHUP == POLLHUP, "");

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

EPoller::EPoller(EventLoop* loop)
    : ownerLoop_(loop),
      epollfd_(::epoll_create(EPOLL_CLOEXEC)),
      events_(kInitEvnetListSize)
{
    if(epollfd_ < 0) {
        printf("LOG_SYSFATAL: EPoller::EPoller\n");
        exit(-1);
    }
}

EPoller::~EPoller() {
    ::close(epollfd_);
}

Timestamp EPoller::poll(int timeoutMs, ChannelList* activeChannels) {
    int numEvents = ::epoll_wait(epollfd_, 
                                &*events_.begin(),
                                static_cast<int>(events_.size()),
                                timeoutMs);
    Timestamp now(Timestamp::now());

    if(numEvents > 0) {
        //printf("LOG_TRACE: %d events happended\n", numEvents);
        fillActiveChannels(numEvents, activeChannels);

        if(implicit_cast<size_t>(numEvents) == events_.size())  {
            events_.resize(events_.size() * 2); //动态扩容
        }
    }
    else if(numEvents == 0) {
        //printf("LOG_TRACE: nothing happended\n");
    }
    else {
        printf("LOG_SYSERR: EPoller::poll()\n");
    }

    return now;
}

void EPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels_) const {
    assert(implicit_cast<size_t>(numEvents) <= events_.size());

    for(int i = 0; i < numEvents; ++i) {
        Channel* channel = static_cast<Channel*> (events_[i].data.ptr);
#ifndef NODEBUG
    int fd = channel->fd();
    auto it = channels_.find(fd);
    assert(it != channels_.end());
    assert(it->second == channel);
#endif
    channel->set_revents(events_[i].events);
    activeChannels_->push_back(channel);
    }
}

void EPoller::updateChannel(Channel* channel){
    assertInLoopThread();
    //printf("LOG_TRACE: fd = %d events = %d\n", channel->fd(), channel->events());
    const int index = channel->index();

    if(index == kNew || index == kDeleted) {
        // a new one, add with EPOLL_CTL_ADD, 注册新的事件
        int fd = channel->fd();
        if(index == kNew) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else { // kDeleted
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
            // and do nothing
        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else {
        // update existing one with EPOLL_CTL_MOD/DEL
        int fd = channel->fd();
        ///(void)fd;
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);

        if(channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}


void EPoller::removeChannel(Channel* channel) {
    assertInLoopThread();
    int fd = channel->fd();
    //printf("LOG_TRACE: removeChannel fd = %d\n", fd);
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());

    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    size_t n = channels_.erase(fd);
    assert(n == 1); (void)n;

    if(index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}


void EPoller::update(int operation, Channel* channel) {
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();

    if(::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if(operation == EPOLL_CTL_DEL) {
            printf("LOG_SYSERR: epoll_ctl op = EPOLL_CTL_DEL, fd = %d", fd );
            exit(-1);
        }
        else {
            printf("LOG_SYSFATAL: epoll_ctl op = %d fd= %d\n", operation, fd);
            exit(-1);
        }
    }
}

