#ifndef SMUDUO_TCP_TCPCLIENT_H
#define SMUDUO_TCP_TCPCLIENT_H

#include "../Base/Common.h"
#include "TcpConnection.h"
#include "InetAddress.h"
#include "Connector.h"

#include <mutex>
#include <memory>

//class Connector;
//typedef std::shared_ptr<Connector> ConnectorPtr;


class TcpClient : smuduo::noncopyable {
public:
    TcpClient(EventLoop* loop, const InetAddress& servAddr);
    ~TcpClient();

    void connect();
    void disconnect();
    void stop();

    TcpConnectionPtr connection() const {
        {
            std::lock_guard<std::mutex> locker(mutex_);
            return connection_;
        }       
    }
    bool retry() const;
    void enableRetry() { retry_ = true; }

    // Set connection callback
    // Not thread safe
    void setConnectionCallback(const ConnectionCallback& cb) {
        connectionCallback_ =  cb;
    }

    // Set message callback, Not thread safe
    void setMessageCallback(const MessageCallback& cb) {
        messageCallback_ = cb;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
        writeCompleteCallback_ = cb;
    }

private:
    // not thread safe, but in loop
    void newConnection(int sockfd);
    // not thread safe, but in loop
    void removeConnection(const TcpConnectionPtr& conn);

    EventLoop* loop_;
    ConnectorPtr connector_; //avoid revealing connector
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    bool retry_; //atomic
    bool connect_;

    // always  in loop thread
    int nextConnId_;
    mutable std::mutex mutex_;
    TcpConnectionPtr connection_; // @BuardedBy mutex_

};



#endif