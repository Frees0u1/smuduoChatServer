#ifndef SMUDUO_TCP_TCPSERVER_H
#define SMUDUO_TCP_TCPSERVER_H


#include "../Base/Common.h"
#include "../Net/Callback.h"
#include "../Net/EventLoopThreadPool.h"

#include "InetAddress.h"
#include "TcpConnection.h"


#include <map>
#include <string>
#include <memory>


/*
*   TcpServer内部使用Acceptor来获得新连接的fd. 它保存用户提供的ConnectionCallback 和 MessageCallback,
* 并在建立连接即TcpConnection对象时原样传给后者. TcpServer持有目前存活的TcpConnection的shared_ptr
*   每个TcpConnection对象有一个名字,这个名字是由其所属的TcpServer在创建TcpConnection对象时生成,名字是ConnectionMap的key值
*   在新连接到达时,Acceptor会回调newConnection(), 后者会创建TcpConnection对象conn, 把它加入ConnectionMap,
* 设置好Callback,再调用conn->connectEstablished(), 其中会回调用户提供的ConnectionCallback
*/

// TODO: add HighWaterCallback: 如果输出缓冲的长度超过用户指定的大小,则触发回调

class Acceptor;
class EventLoop;


class TcpServer : smuduo::noncopyable {
public:
    TcpServer(EventLoop* loop, const InetAddress& listenAddr); //由listenAddr建立的一个TcpServer对象
    ~TcpServer();
    
    // 设定初始的线程池大小
    //
    // 总是在TcpServer的所在的主线程中接受新连接
    // @param numThreads
    // -0 意味着所有的I/O都在本线程中,没有新建的线程, [-默认值-]
    // -1 所有的I/O操作都在另外一个线程中完成
    // -N 意味着创建一个有N个线程所组成的线程池,每一个线程都被注册在一个
    //    round-robin basics(轮询)
    void setThreadNum(int numThreads);


    /// Starts the server if it's not running
    ///
    /// harmless to call it multiple times. thread safe
    void start();

    /// Set connection Callback. Not thread safe
    void setConnectionCallback(const ConnectionCallback& cb) {
        connCallback_ = cb;
    }

    /// Set message Callback, Not thread Safe
    void setMessageCallback(const MessageCallback& cb) {
        msgCallback_ = cb;
    }

    /// Set write complete callback.
    /// Not thread safe.
    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }

private:
    /// Not thread safe, but in loop
    void newConnection(int sockfd, const InetAddress& peerAddr); //指定地址的指定socket有新事件发生
    // Thread Safe
    void removeConnection(const TcpConnectionPtr& conn);
    // Not thread safe but in loop
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap; // key为TcpConnection的name_, value为指向TcpConnection对象的智能指针

    EventLoop* loop_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_; // avoid revealing Acceptor
    std::unique_ptr<EventLoopThreadPool> threadPool_;
    ConnectionCallback connCallback_;
    MessageCallback msgCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    bool started_;
    int nextConnId_; //always in loop thread
    ConnectionMap connections_;
};


#endif