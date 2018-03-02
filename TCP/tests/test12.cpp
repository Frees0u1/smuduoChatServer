#include "../Connector.h"
#include "../../Net/EventLoop.h"
#include "../InetAddress.h"


EventLoop* g_loop;

//int connCnt; //连接次数

void connectCallback(int sockfd) {
    printf("connected.\n");
    g_loop->quit(); //退出循环
}

int main(int argc, char** argv) {
    EventLoop loop;
    g_loop = &loop;
    InetAddress addr("127.0.0.1", 9981);
    ConnectorPtr connector(new Connector(&loop, addr));
    connector->setNewConnectionCallback(connectCallback);
    connector->start();

    loop.loop();
}