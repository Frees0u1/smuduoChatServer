#include "../../Net/EventLoop.h"
#include "../InetAddress.h"
#include "../TcpClient.h"

#include <functional>
#include <utility>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <iostream>

using namespace std;
std::string message;

void onConnection(const TcpConnectionPtr& conn) {
    if(conn->connected()) {
        printf("onConncetion(): new connection [%s] from %s\n",
                conn->name().c_str(), conn->peerAddress().toIpPort().c_str());
        conn->send(message);
    }
    else {
        printf("onConnection(): connection [%s] is down\n", conn->name().c_str());
    }
    std::getline(std::cin, message); // cin a sting with space
    conn->send(message);
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp recvTime) {
    printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
    buf->readableBytes(), conn->name().c_str(),
    recvTime.toShortTZtime().c_str());
    printf("onMessage(): [%s]\n", buf->retrieveAllAsString().c_str());

    std::getline(std::cin, message);
    conn->send(message);
}


int main() {
    EventLoop loop;
    InetAddress serverAddr("localhost", 9981);
    TcpClient client(&loop, serverAddr);

    client.setConnectionCallback(onConnection);
    client.setMessageCallback(onMessage);
    client.enableRetry();
    client.connect();

    loop.loop();
}