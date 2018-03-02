#include "../Net/EventLoop.h"

#include "TcpServer.h"
#include "Acceptor.h"
#include "SocketOps.h"

#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory>

using namespace smuduo;
using namespace std::placeholders;  // 对于 _1, _2, _3...

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr)
    :   loop_(loop),
        name_(listenAddr.toIpPort()),
        acceptor_(new Acceptor(loop, listenAddr)),
        threadPool_(new EventLoopThreadPool(loop)),
        started_(false),
        nextConnId_(1) 
{
    //CHECK_NOTNULL
    if(loop_ == NULL) {
        printf("LOG_FATAL: TcpServer() loop_ was given with a nullptr!\n");
        exit(-1);
    }
    acceptor_-> setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, _1, _2)
    );
}

TcpServer::~TcpServer() { 
    //为了使unique自动析构,显式写出~TcpServer
}

void TcpServer::setThreadNum(int numThreads) {
    assert(0 <= numThreads);
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() {
    if(!started_) {
        started_ = true;
        threadPool_->start();
    }

    if(!acceptor_->listenning()) {
        loop_->runInLoop(
            std::bind(&Acceptor::listen, acceptor_.get())
        );
    }

}


void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    loop_->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof buf, "#%d", nextConnId_);
    ++nextConnId_;

    std::string connName = name_ + buf;

    printf("LOG_INFO: TcpServer::newConnection [%s] - new connection [%s] from %s\n", name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    EventLoop* ioLoop = threadPool_->getNextLoop();
    //FIXME: poll with zero timeout to double confirm the newconnection
    TcpConnectionPtr conn(
       std::make_shared<TcpConnection>(ioLoop, connName, sockfd, localAddr, peerAddr)
    );

    connections_[connName] = conn; 
    conn->setConnectionCallback(connCallback_);
    conn->setMessageCallback(msgCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this, _1)
    );

    ioLoop->runInLoop(
        std::bind(&TcpConnection::connectEstablished, conn)
    );
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    // FIXME: unsafe
    loop_->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn)
    );
}


void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    loop_->assertInLoopThread();
    printf("LOG_INFO: TcpServer::removeConnectionInLoop [%s] - connection %s\n", name_.c_str(), conn->name().c_str());

    size_t n =  connections_.erase(conn->name());
    assert( n == 1); (void)n;

    EventLoop* ioLoop = conn->getLoop();

    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn)
    );

}

void TcpConnection::setTcpNoDelay(bool on) {
    socket_->setTcpNoDelay(on);
}