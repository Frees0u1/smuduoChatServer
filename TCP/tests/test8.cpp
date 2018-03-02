#include "../TcpServer.h"
#include "../TcpConnection.h"


#include "../../Net/EventLoop.h"

#include <iostream>

using namespace std;

void onConnection(const TcpConnectionPtr& conn) {
    if(conn->connected() ){
        printf("onConnnection(): new connection [%s] from %s\n",
                conn->name().c_str(), conn->peerAddress().toIpPort().c_str());
    }
    else { //通知用户连接已经关闭
        printf("onConnection(): connection [%s] is down\n",
                conn->name().c_str());
    }
}

void onMessage(const TcpConnectionPtr& conn, const char* data, ssize_t len) {
    printf("onMessage(): received %zd bytes from connection [%s]:[%s]\n",
            len, conn->name().c_str(), data);

    for(int i = 0; i < len; i++){
        printf("[%d] ", data[i]);
    }
    printf("\n");
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