#include "../TcpServer.h"
#include "../../Net/EventLoop.h"
#include "../InetAddress.h"
#include <iostream>

using namespace std;

void onConnection(const TcpConnectionPtr& conn) {
    if(conn->connected()) {
        printf("onConnection(): new connection [%s] from %s\n",
                conn->name().c_str(), conn->peerAddress().toIpPort().c_str());
    }
    else {
        printf("onConnection: connection [%s] is down\n",
                conn->name().c_str());

    }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime) {
    printf("onMessage(): receive %zd bytes from connection [%s] at %s\n",
            buf->readableBytes(), conn->name().c_str(),
            receiveTime.toShortTZtime().c_str());
    conn->send("Msg from server at " + Timestamp::now().toShortTZtime() + ":\t" + 
              buf->retrieveAllAsString());
}


int main() {
    printf("main(): pid = %d\n", getpid());
    InetAddress listenAddr(9981);
    EventLoop loop;

    TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();

    loop.loop();

    return 0;
}