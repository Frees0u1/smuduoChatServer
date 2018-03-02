#include "../TcpServer.h"
#include "../../Net/EventLoop.h"
#include "../InetAddress.h"
#include <iostream>
#include <string>

std::string msg1;
std::string msg2;

void onConnection(const TcpConnectionPtr& conn) {
    if(conn->connected()) {
        printf("onConnection(): new connection [%s] from %s\n",
                conn->name().c_str(),
                conn->peerAddress().toIpPort().c_str());
        conn->send(msg1);
        conn->send(msg2);
        conn->shutdown();
    }
    else {
        printf("onConnection(): connection [%s] os down\n",
                conn->name().c_str());
    }
}

void onMessage(const TcpConnectionPtr& conn,
                Buffer* buf,
                Timestamp receiveTime ) {
    // Do Nothing MessageCallback

    printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
          buf->readableBytes(), conn->name().c_str(),
          receiveTime.toFormattedString().c_str());
    buf->retrieveAll();
}



int main(int argc, char **argv) {
    printf("main(): pid = %d\n", getpid());

    int len1 = 100;
    int len2 = 200;

    if(argc == 3) {
        len1 = atoi(argv[1]);
        len2 = atoi(argv[2]);
    }

    msg1.resize(len1);
    msg2.resize(len2);

    std::fill(msg1.begin(), msg1.end(), 'A');
    std::fill(msg2.begin(), msg2.end(), 'B');

    InetAddress listenAddr(9981);
    EventLoop loop;

    TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();

    loop.loop();
}