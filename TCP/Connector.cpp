#include "Connector.h"
#include "SocketOps.h"
#include <functional>
#include <errno.h>
#include  <memory>
using namespace smuduo;

const int Connector::kMaxRetryDelayMs;
const int Connector::kInitRetryDelayMs;


Connector::Connector(EventLoop* loop, const InetAddress& serverAddr) 
    : loop_(loop),
      serverAddr_(serverAddr),
      connect_(false),
      state_(kDisconnected),
      retryDelayMs_(kInitRetryDelayMs)
{
    printf("LOG_DEBUG: ctor [%p] \n", this);
}

Connector::~Connector() {
    printf("LOG_DEBUG: dtor [%p] \n", this);
    loop_->cancel(timerId_);
    assert(!channel_);
}

void Connector::start() {
    connect_  = true;
    loop_->runInLoop(
        std::bind(&Connector::startInLoop, this) //FIXME: unsafe
    );
}

void Connector::startInLoop() {
    loop_->assertInLoopThread();
    assert(state_ == kDisconnected);
    if(connect_) {
        connect(); 
    }
    else {
        printf("LOG_DEBUG: do not connect\n");
    }
}


void Connector::connect() { //创建sockfd, 并尝试建立连接
    int sockfd = sockets::createNonBlockingOrDie();
    int ret = sockets::connect(sockfd, serverAddr_.getSockAddrInet());

    int savedErrno = (ret == 0) ? 0 : errno;

    switch(savedErrno) {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            connecting(sockfd);
            break;
        
        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry(sockfd);
            break;
        
        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            printf("LOG_SYSERR: connect error in Connect::startInLoop %d\n", savedErrno);
            sockets::close(sockfd);
        
        default:
            printf("LOG_SYSERR: Unexpected error in Connector::startInLoop %d\n", savedErrno);
            sockets::close(sockfd);
            break;
    }
}

void Connector::restart() {
    loop_->assertInLoopThread();
    setState(kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}

void Connector::stop() {
    connect_ = false;
    loop_->cancel(timerId_);
}

void Connector::connecting(int sockfd) {
    setState(kConnecting);
    assert(!channel_);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->setWriteCallback(
        std::bind(&Connector::handleWrite,  this)
    );
    channel_->setErrorCallback(
        std::bind(&Connector::handleError, this)
    );

    channel_->enableWriting();
}

int Connector::removeAndResetChannel() {
    channel_->disableAll();
    loop_->removeChannel(channel_.get());
    
    int sockfd = channel_->fd();
    // Can't reset channel_ here, because we are inside Channel::handleEvent
    loop_->queueInLoop(
        std::bind(&Connector::resetChannel, this)
    );
    return sockfd;
}


void Connector::resetChannel() {
    channel_.reset();
}

void Connector::handleWrite() {
    //printf("LOG_TRACE: Connector::handleWrite state = %d\n", state_);

    if(state_ == kConnecting) {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        if(err) {
            //printf("LOG_WARN: Connector::handleWrite - SO_ERROR = %d", err);
            retry(sockfd);
        }
        else if(sockets::isSelfConnect(sockfd)) {
            //printf("LOG_WARN Connector::handleWrite - Self connect");
        }
        else {
            setState(kConnected);
            if(connect_) {
                newConnectionCallback_(sockfd); // 连接建立后,进行处理
            }
            else {
                sockets::close(sockfd);
            }
        }
    }
    else {
        assert(state_ == kDisconnected);
        // What happened?
    }
}

void Connector::handleError() {
    //printf("LOG_ERROR Connector::handlError\n");
    assert(state_ == kConnecting);

    int sockfd = removeAndResetChannel();
    int err = sockets::getSocketError(sockfd);
    //printf("LOG_TRACE: SO_ERROR = %d\n", err);
    retry(sockfd);
}

void Connector::retry(int sockfd) {
    sockets::close(sockfd);
    setState(kDisconnected);
    if(connect_) {
        printf("LOG_INFO Connector::retry - Retry connecting to %s in %d ms.\n",
                serverAddr_.toIpPort().c_str(),
                retryDelayMs_);
        timerId_ = loop_->runAfter(
            retryDelayMs_/1000.0,
            std::bind(&Connector::startInLoop, this)
        );
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    }
    else {
        printf("LOG_DEBUG: do not connect");
    }
}



