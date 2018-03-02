#include "../Net/Channel.h"
#include "../Net/EventLoop.h"

#include "Socket.h"
#include "SocketOps.h"
#include "TcpConnection.h"
#include <functional>
#include <errno.h>
#include <string>
#include <stdio.h>
#include <assert.h>

using namespace smuduo;
using namespace std::placeholders;

TcpConnection::TcpConnection(EventLoop* loop,
                            const std::string& nameArg,
                            int sockfd,
                            const InetAddress& localAddr,
                            const InetAddress& peerAddr)
    :   loop_(loop),
        name_(nameArg),
        state_(kConnecting),
        socket_(std::make_unique<Socket>(sockfd)),
        channel_(std::make_unique<Channel>(loop, sockfd) ),
        localAddr_(localAddr),
        peerAddr_(peerAddr)
{
    assert(loop_ != nullptr);
//    printf("LOG_INFO: TcpConnection::ctor[%s] at %p fd=%d\n", name_.c_str(), this, sockfd);
    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead, this, _1)
    );
    channel_->setWriteCallback(
        std::bind(&TcpConnection::handleWrite,  this)
    );
    channel_->setCloseCallback(
        std::bind(&TcpConnection::handleClose, this)
    );
    channel_->setErrorCallback(
        std::bind(&TcpConnection::handleError,  this)
    );
}

TcpConnection::~TcpConnection() {
 //   printf("LOG_INFO: TcpConnection::dtor[%s] at %p fd=%d\n", name_.c_str(), this, channel_->fd());
}

void TcpConnection::send(const void* data, size_t len) {
    send(std::string(static_cast<const char*>(data), len));
}


void TcpConnection::send(const std::string& message) {
    if(state_ == kConnected) {
        if(loop_->isInLoopThread()) {
            sendInLoop(message.data(), message.size());
        } else {
            loop_->runInLoop(
                std::bind(&TcpConnection::sendInLoop, this, message.data(), message.size())
            );
        }
    }
}

void TcpConnection::send(std::shared_ptr<Buffer> buf) {
    int len = buf->peekInt32();
    // printf("===DEBUG: tid = %d, TcpConnection::send(Buffer* version): %d ====\n", CurrentThread::tid(), len);
    // const char* data = buf->peek();
    // printf("===DEBUF: Buffer=");
    // for(int i = 4; i < buf->readableBytes(); i++){
    //     printf("%c", *(static_cast<const char*>(data)+i));
    // }
    // printf("\n");

    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoopBuffer(buf);
            buf->retrieveAll();
        }
        else {  // 去另外一个线程中执行,这里可能会有提前消亡,所有应该传值      
            loop_->runInLoop(
                std::bind(&TcpConnection::sendInLoopBuffer,
                            this,
                            buf)
            );
            //buf->retrieveAll();
        }
    }
}




// void TcpConnection::sendInLoop(const std::string& message) {
//     sendInLoop(message.data(), message.size());
// }
void TcpConnection::sendInLoopBuffer(std::shared_ptr<Buffer> buf) {
    std::shared_ptr<Buffer> buftmp = buf; //阻止buf析构
    sendInLoop(buf->peek(), buf->readableBytes());
//    buf->retrieveAll();// 有没有也无所谓吧,反正要消除
    buf.reset();
}

void TcpConnection::sendInLoop(const void* data, size_t len) {
    int32_t be32 = *static_cast<const int32_t*>(data); // SIGBUS
    const int32_t l = sockets::networkToHost32(be32);
    // printf("===DEBUG: tid = %d, TcpConnection::sendInLoop(void* version): %d ====\n", CurrentThread::tid(), l);
    // printf("===DEBUG: Buffer=");
    // for(int i = 4; i < len; i++){
    //     printf("%c", *(static_cast<const char*>(data)+i));
    // }
    // printf("\n");

    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;

    if(state_ == kDisconnected) {
        printf("LOG_WARN: disconnected, give up writing");
        return;
    }


    // if no thing in output queue, try writing directly
    if(!channel_->isWriting() & outputBuffer_.readableBytes() == 0){
        nwrote = ::write(channel_->fd(), static_cast<const char*>(data), len);
        if(nwrote >= 0) {
           remaining = len - nwrote;
           if(remaining == 0 && writeCompleteCallback_) {
                loop_->queueInLoop(
                    std::bind(writeCompleteCallback_, shared_from_this())
                );
            }
        }
        else { // nwrote < 0
            nwrote = 0;
            if(errno != EWOULDBLOCK) {
                printf("LOG_SYSERR TcpConnection::sendInLoop");
            }
            if(errno == EPIPE ||  errno == ECONNRESET) {
                // FIXME: any other?
                faultError = true;
            }
        }
    }

    assert(nwrote <= len);

    if(!faultError && implicit_cast<size_t>(nwrote) < len) {
        outputBuffer_.append(static_cast<const char*>(data) + nwrote, remaining);
        if(!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::handleRead(Timestamp receiveTime) {
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);


    if(n > 0) {
        msgCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    } else if(n == 0) {
        handleClose();
    } else {
        errno = savedErrno;
    //    printf("LOG_SYSERR TcpConnection::handleRead");
        handleError();
    }
}

void TcpConnection::handleWrite() {
    loop_->assertInLoopThread();
    if(channel_ -> isWriting()) {
        ssize_t n = ::write(
            channel_->fd(),
            outputBuffer_.peek(),
            outputBuffer_.readableBytes()
        );
        if(n > 0) {
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if(writeCompleteCallback_) {
                    loop_->queueInLoop(
                        std::bind(writeCompleteCallback_, shared_from_this())
                    );
                }
                if(state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            } else {
                //printf("LOG_TRACE: smuduo is going to write more data\n");
            } 
        } else {
            //printf("LOG_TRACE: Connection is down, no more writing\n");
        }

    }

}



void TcpConnection::handleClose() {
    loop_ -> assertInLoopThread();
    //printf("LOG_TRACE: TcpConnection::handleClose state = %d\n", state_);
    assert(state_ == kConnected || state_ == kDisconnecting); 
    // dont close the fd, leave  it to dtor, so we can find leaks easily
    channel_->disableAll();
    // must be the last line
    closeCallback_(shared_from_this());
}

// handleError() 并没有进一步行动,只是在日志中输出错误消息,这不影响连接的正常关闭
void TcpConnection::handleError() {
    int err = sockets::getSocketError(channel_->fd());
    // FIXME!!! 需要写一个strerror_r函数将errno转化成对应的错误字符串
    printf("LOG_ERROR: TcpConnection::handleError [%s] - SO_ERROR = %d ", name_.c_str(), err);
}


void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->enableReading();

    connCallback_(shared_from_this());
}


// connectDestroted()是TcpConnection析构前调用的最后一个函数,它通知用户连接已断开

void TcpConnection::connectDestroyed() {
    loop_->assertInLoopThread();
    assert(state_ == kConnected || state_ == kDisconnecting);
    setState(kDisconnected);
    channel_->disableAll();
    connCallback_(shared_from_this());

    loop_->removeChannel(channel_.get());
}

void TcpConnection::shutdown() {
    // FIXME: use compare and swap
    if(state_  == kConnected) {
        setState(kDisconnecting);
        //FIXME: shared_from_this()?
        loop_->runInLoop(
            std::bind(&TcpConnection::shutdownInLoop,  this)
        );
    }
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    if(!channel_->isWriting()) {
        // we are not writing
        socket_->shutdownWrite();
    }
}

