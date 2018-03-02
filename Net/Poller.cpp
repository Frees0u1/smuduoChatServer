#include "Poller.h"
#include "Channel.h"
#include "EventLoop.h"
#include <poll.h>
#include <cstdio>
#include <assert.h>

Poller::Poller(EventLoop *loop)
    : ownerLoop_(loop)
{    }

Poller::~Poller() {

}

Timestamp Poller::poll(int timeoutMs, ChannelList* activeChannels) {
    int numEvents = ::poll(pollfds_.data(), pollfds_.size(), timeoutMs);
    Timestamp now(Timestamp::now());
  //  printf("LOG_DEBUG: current timerQueue size is %lu", ownerLoop_->currentTQsize());

    if(numEvents > 0) {
        //printf("LOG_TRACE: %d events happened\n", numEvents);
        fillActiveChannels(numEvents, activeChannels);
    } else if (numEvents == 0) {
        //printf("LOG_TRACE: nothing happened!\n");
    } else {
        printf("LOG_SYSERR: Poller::poll()");
    }
    return now;
}

void Poller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const {
    for(PollFdList::const_iterator pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0; ++pfd) {
        if(pfd->revents) {
            --numEvents;
            ChannelMap::const_iterator ch = channels_.find(pfd->fd);
            assert(ch != channels_.end());
            Channel* channel = ch->second;
            assert(channel->fd() == pfd->fd);
            channel->set_revents(pfd->revents);

            activeChannels->push_back(channel);
        }
    }
}

// updateChannel()的主要功能是负责维护和更新pollfds_数组。添加新Channel的复杂度是O(logN),更新已有的Channel的复杂度是O(1),
// 因为Channel记住了自己在pollfds_数组中的下标，因此可以快速定位，removeChannel()的复杂度也将会是O(logN)

void Poller::updateChannel(Channel* channel) {
    assertInLoopThread();
    //printf("LOG_TRACE: Poller() updateChannel: fd = %d events = %d\n", channel->fd(), channel->events());

    if(channel->index() < 0) { // a new one, add to pollfds_
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        int idx = static_cast<int>(pollfds_.size()) - 1;
        channel->set_index(idx);
        channels_[pfd.fd] = channel;
    }
    else { //update existing one
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);

        int idx = channel->index(); 
        assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));

        struct pollfd& pfd = pollfds_[idx];
        assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if(channel -> isNoneEvent()) {
            //ignore this pollfd
            pfd.fd = - channel->fd() -  1; //see explain in p.286~p.287
        }
    }
}

void Poller::removeChannel(Channel* channel) {
    assertInLoopThread();
    //printf("LOG_TRACE: Poller::removeChannel(): fd = %d\n", channel->fd());
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    assert(channel->isNoneEvent());

    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));

    const struct pollfd& pfd = pollfds_[idx]; (void)pfd;
    assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());
    size_t n = channels_.erase(channel->fd());
    assert(n == 1); (void)n;

    if(implicit_cast<size_t>(idx) == pollfds_.size() - 1) {
        pollfds_.pop_back();
    } else {
        int channelAtEnd = pollfds_.back().fd;
        iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
        if(channelAtEnd < 0) {
            channelAtEnd = -channelAtEnd - 1;
        }
        channels_[channelAtEnd]->set_index(idx);
        pollfds_.pop_back();
    }
}