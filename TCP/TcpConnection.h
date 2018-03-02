#ifndef SMUDUO_TCP_TCPCONNECTION_H
#define SMUDUO_TCP_TCPCONNECTION_H

// TcpConnection表示的是"一次TCP连接",它是不可再生的,一旦连接断开,这个TcpConnection对象
// 也就没啥用了.另外TcpConnection没有发起连接的功能,其构造函数的sockfd是已连接的fd
// 无论是主动发起还是被动接受,因此其初始状态是kConnecting


#include "../Net/Callback.h"
#include "../Base/Common.h"
#include "../Net/Buffer.h"
#include "InetAddress.h"

#include <memory>

class Channel;
class EventLoop;
class Socket;


//using namespace smuduo;

class TcpConnection : smuduo::noncopyable,
                      public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop,
                const std::string& name,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() { return localAddr_; }
    const InetAddress& peerAddress() { return peerAddr_; }
    bool connected() const { return state_ == kConnected; }

    void setConnectionCallback(const ConnectionCallback& cb)
    { connCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { msgCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }

    //仅限内部使用
    // 当TcpServer接收到一个新连接时被调用,应该只被调用一次
    void connectEstablished();

    void setCloseCallback(const CloseCallback& cb) {
        closeCallback_ = cb;
    }
    // called when TcpServer has removed me from its map
    void connectDestroyed(); // should be called only once

    void send(const void* message, size_t len);
    void send(const std::string& message);
    //void send(Buffer* buf);
    void send(std::shared_ptr<Buffer> buf);
    // Thread Safe
   
    // Thread Safe
    void shutdown();
    void setTcpNoDelay(bool on);

private:
    enum StateE {kConnecting, kConnected, kDisconnecting, kDisconnected, };

    void setState(StateE s) { state_ = s; }
    void handleRead(Timestamp receiveTime); //会检查read的返回值,根据返回值分别调用msgCallback_, handleClose()和handleError()
    void handleWrite();
    void handleClose();
    void handleError();

//    void sendInLoop(const std::string&  message);
    void sendInLoop(const void*, size_t len);
    void sendInLoopBuffer(std::shared_ptr<Buffer> buf); //指向堆区的buf,用智能指针管理
    void shutdownInLoop();

    EventLoop* loop_;
    std::string name_;
    StateE state_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    InetAddress localAddr_;
    InetAddress peerAddr_;
    ConnectionCallback connCallback_;
    MessageCallback msgCallback_;
    CloseCallback  closeCallback_;
    WriteCompleteCallback writeCompleteCallback_;


    Buffer inputBuffer_;
    Buffer outputBuffer_;
};

#endif