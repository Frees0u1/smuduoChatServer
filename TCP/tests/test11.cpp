// chargen 服务,不间断发送服务

#include "../TcpServer.h"
#include "../../Net/EventLoop.h"
#include "../InetAddress.h"
#include <stdio.h>

std::string message;

void onConnection(const TcpConnectionPtr&  conn) {
    if(conn->connected()) {
        printf("onConnection(): tid=%d new connection [%s] from %s\n",
                CurrentThread::tid(),
                conn->name().c_str(),
                conn->peerAddress().toIpPort().c_str());
        conn->send(message); //发送初始消息
    }
    else {
        printf("onConnection(): tid=%d connection [%s] is down\n",
                CurrentThread::tid(),
                conn->name().c_str());
    }
}

void onWriteComplete(const TcpConnectionPtr& conn) {
    printf("onWriteComplete() send a message to connection %s\n", conn->name().c_str());
    conn->send(message);
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp recvTime) {
    printf("onMessage(): tid=%d received %zd bytes from connection [%s] at %s\n",
            CurrentThread::tid(),
            buf->readableBytes(),
            conn->name().c_str(),
            recvTime.toFormattedString().c_str());
    buf->retrieveAll();
}

int main(int argc, char* argv[]) {
    printf("main(): pid = %d\n", getpid());

    std::string line;
    for(int i = 33; i < 127; ++i) {
        line.push_back(char(i));
    }
    line += line;

    for(size_t i = 0; i < 127 - 33; ++i) {
        message += line.substr(i, 72) + '\n';
    }

    InetAddress listenAddr(9981);
    EventLoop loop;

    TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.setWriteCompleteCallback(onWriteComplete);

    // 第二个参数控制线程池大小
    if(argc > 1) {
        server.setThreadNum(atoi(argv[1]));
    }
    server.start();

    loop.loop();

    return 0;
} 