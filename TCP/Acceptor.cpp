#include "Acceptor.h"
#include "SocketOps.h"
#include <functional>

using namespace smuduo;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr) 
:  loop_(loop),
    acceptSocket_(sockets::createNonBlockingOrDie()),
    acceptChannel_(loop, acceptSocket_.fd()),
    listenning_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.bindOrDie(listenAddr);
   
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

void Acceptor::listen() {
    loop_->assertInLoopThread();
    listenning_ = true;
    acceptSocket_.listenOrDie(); //监听开始
    acceptChannel_.enableReading();
}


void Acceptor::handleRead() {
    loop_->assertInLoopThread();
    InetAddress peerAddr(0);
    
    //FIXME loop until no more
    int connfd = acceptSocket_.accept(&peerAddr);

    if(connfd >= 0) { // must be, or will be aborted
        if(newConnectionCallback_) {
            newConnectionCallback_(connfd, peerAddr);
        } else {
            sockets::close(connfd); //没有设定回调的新连接，直接关闭
        }
    }
}