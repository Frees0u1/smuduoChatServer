#include "../Acceptor.h" 
#include "../SocketOps.h"
#include "../InetAddress.h"

#include "../../Net/EventLoop.h"


void newConnection1(int sockfd, const InetAddress& peerAddr) {
    printf("newConnection(): accepted a new connection from %s",
           peerAddr.toIpPort().c_str());
    ::write(sockfd, "How are you?\n", 13);
    smuduo::sockets::close(sockfd);
}

void newConnection2(int sockfd, const InetAddress& peerAddr) {
    printf("newConnection(): accepted a new connection from %s",
           peerAddr.toIpPort().c_str());
    ::write(sockfd, "I am fine.\n", 11);
    smuduo::sockets::close(sockfd);
}

int main() {
    printf("main() pid = %d\n", getpid());

    InetAddress listenAddr1(9981); //在9981端口侦听新连接
    InetAddress listenAddr2(9982); //在9982端口侦听新连接
    EventLoop loop;

    Acceptor acceptor1(&loop, listenAddr1);
    Acceptor acceptor2(&loop, listenAddr2);
    acceptor1.setNewConnectionCallback(newConnection1);
    acceptor2.setNewConnectionCallback(newConnection2);
    acceptor1.listen(); //开始侦听
    acceptor2.listen();


    loop.loop();

    return 0;
}