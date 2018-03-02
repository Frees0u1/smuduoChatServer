#include "../Acceptor.h" 
#include "../SocketOps.h"
#include "../InetAddress.h"

#include "../../Net/EventLoop.h"


void newConnection(int sockfd, const InetAddress& peerAddr) {
    printf("newConnection(): accepted a new connection from %s",
           peerAddr.toIpPort().c_str());
    ::write(sockfd, "How are you?\n", 13);
    smuduo::sockets::close(sockfd);
}

int main() {
    printf("main() pid = %d\n", getpid());

    InetAddress listenAddr(9981); //在9981端口侦听新连接
    EventLoop loop;

    Acceptor acceptor(&loop, listenAddr);
    acceptor.setNewConnectionCallback(newConnection);
    acceptor.listen(); //开始侦听

    loop.loop();

    return 0;
}