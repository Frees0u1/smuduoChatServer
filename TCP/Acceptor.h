#ifndef SMUDUO_TCP_ACCEPTOR_H
#define SMUDUO_TCP_ACCEPTOR_H 

#include "../Base/Common.h"
#include "../Net/EventLoop.h"
#include "../Net/Channel.h"

#include "Socket.h"

#include <functional>



/* 
 * Acceptor使用::accept() 服务器端接收新的TCP连接，并通过回调通知使用者。
 * 是内部class,供TcpServer使用，生命期由后者控制
 */

class InetAddress;
class Acceptor : smuduo::noncopyable {
public:
    typedef std::function<void (int sockfd, const InetAddress&)> NewConnectionCallback;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr); //在EventLoop中接收一个新的连接

    void setNewConnectionCallback(const NewConnectionCallback& cb) {
        newConnectionCallback_ = cb;
    }

    bool listenning() const { return listenning_; }
    void listen();

private:
    void handleRead();

    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;


};

#endif