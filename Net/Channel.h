#ifndef SMUDUO_NET_CHANNEL_H
#define SMUDUO_NET_CHANNEL_H

//#include "EventLoop.h"



#include "../Base/Common.h"
#include "../Base/Timestamp.h"
#include <memory>
#include <string>
#include <string.h>
#include <functional>
#include <utility> //for std::move


class EventLoop;

/*
每个Channel对象自始至终只属于一个EventLoop,因此每个Channel对象都只属于某一个IO线程。每个Channel对象自始至终只负责一个文件描述符(fd)的IO事件分发，
但它并不拥有这个fd,也不会在析构的时候关闭这个fd. Channel是fd的管理者
Channel会把不同的IO事件分成不同的回调，例如ReadCallback, WriteCallback等。而且回调用boost::function表示（好像C++11也可以？）,用户无需继承Channel,
 Channel不是基类。smuduo用户不需要直接使用Channel，而会使用更上层的封装，如TcpConnection
Channel的生命期由owner class负责管理，它一般是其他class的直接或间接成员
*/

class Channel : smuduo::noncopyable {    
public:
    typedef std::function<void()> EventCallback;
    typedef std::function<void(Timestamp)> ReadEventCallback;

 //   typedef std::function<void(Timestamp)> ReadEventCallback;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handleEvent(Timestamp receiveTime);
    void setReadCallback(const ReadEventCallback &cb) {
        readCallback_ = cb;
    }
    void setWriteCallback(const EventCallback &cb) {
        writeCallback_ = cb;
    }
    void setCloseCallback(const EventCallback &cb) {
        closeCallback_ = cb;
    }
    void setErrorCallback(const EventCallback &cb) {
        errorCallback_ = cb;
    }
    
    void setReadCallback(const ReadEventCallback &&cb) {
        readCallback_ = std::move(cb);
    }
    void setWriteCallback(const EventCallback &&cb) {
        writeCallback_ = std::move(cb);
    }
    void setCloseCallback(const EventCallback &&cb) {
        closeCallback_ = std::move(cb);
    }
    void setErrorCallback(const EventCallback &&cb) {
        errorCallback_ = std::move(cb);
    }

    //Tie this channel to the owner object managed by shared_ptr
    // prevent the owner object beging destroyed in handleEvent
    // void tie(const std::shared_ptr<void>&);

    int fd() const {return fd_;}
    int events() const {return events_; }
    void set_revents(int revt) {revents_ = revt; } //used by pollers
    bool isNoneEvent() const {return events_ == kNoneEvent; }

    void enableReading() {
        events_ |= kReadEvent; update();
    }
    void disableReading() {
        events_ &= ~kReadEvent; update();
    }
    void enableWriting() {
        events_ |= kWriteEvent; update();
    }
    void disableWriting() {
        events_ &= ~kWriteEvent; update();
    }
    void disableAll() {
        events_ = kNoneEvent; update();
    }
    bool isWriting() const {
        return events_ & kWriteEvent;
    }

    //For Poller
    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    //for debug
  //  std::string reventsToString() const;

  //  void doNotLogHup() { logHup_  = false; }

    EventLoop* ownerLoop() {return loop_;}
   // void remove();



private:
    void update();
    void handleEventWithGuard();

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;
    const int fd_;
    int events_;
    int revents_; //it's the received event types of epoll or poll
    int index_; //used by Poller

  //  std::weak_ptr<void> tie_;
  //  bool tied_;
  //  bool eventHanding_;
  //  bool addToLoop_;

    bool eventHandling_;

    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;

};


#endif