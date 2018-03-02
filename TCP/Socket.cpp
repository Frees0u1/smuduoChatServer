#include "Socket.h"
#include "InetAddress.h"
#include "../Base/Common.h"
#include "SocketOps.h"

#include <assert.h>
#include <strings.h> //bzero
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

#include <stdio.h> // perror()
#include <stdlib.h> // abort()

using namespace smuduo;

namespace{

    typedef struct sockaddr SA;
    const SA* sockaddr_cast(const struct sockaddr_in* addr){
        return static_cast<const SA*>(implicit_cast<const void*>(addr) );
    }
    SA* sockaddr_cast(struct sockaddr_in* addr){
        return static_cast<SA*>(implicit_cast<void *>(addr));
    }
}

Socket::Socket(int sockfd)
  : sockfd_(sockfd) {
      assert(sockfd_ >= 0);
}

Socket::~Socket(){
    if (sockfd_ >= 0){
        int ret = ::close(sockfd_);
        assert(ret == 0); (void)ret;
    }
}

void Socket::bindOrDie(const InetAddress& addr){
    const struct sockaddr_in& saddr = addr.getSockAddrInet();
    int ret = ::bind(sockfd_, sockaddr_cast(&saddr), sizeof saddr);
    if(ret){
        perror("Socket::bindOrDie");
        abort();
    }
}

void Socket::listenOrDie(){
    int ret = ::listen(sockfd_, SOMAXCONN);
    if(ret){
        perror("Socket::listenOrDie");
        abort();
    }
}

int Socket::accept(InetAddress* peeraddr)
{
  struct sockaddr_in addr;
  bzero(&addr, sizeof addr);
  int connfd = sockets::accept(sockfd_, &addr);
  peeraddr->setSockAddrInet(addr);

  return connfd;
}


int Socket::connect(const InetAddress& addr){
    const struct sockaddr_in& saddr = addr.getSockAddrInet();
    return ::connect(sockfd_, sockaddr_cast(&saddr), sizeof saddr);
}

void Socket::shutdownWrite(){
    if(::shutdown(sockfd_, SHUT_WR) < 0){
        perror("Socket::shutdownWrite");
    }
}

void Socket::setReuseAddr(bool on){
    int optval = on ? 1 : 0;
    if(::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                    &optval, static_cast<socklen_t>(sizeof optval)) < 0)
    {
        perror("Socket::setReuseAddr");
    }
}

void Socket::setTcpNoDelay(bool on){
    int optval = on ? 1 : 0;
    if(::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
                    &optval, static_cast<socklen_t>(sizeof optval)) < 0)
    {
        perror("Socket::setTcpNoDelay");
    }
    // FIXME: CHECK
}

InetAddress Socket::getLocalAddr() const {
    struct sockaddr_in localaddr;
    bzero(&localaddr, sizeof localaddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
    if(::getsockname(sockfd_, sockaddr_cast(&localaddr), &addrlen) < 0)
    {
        perror("Socket::getLocalAddr");
    }
    return InetAddress(localaddr);
}

InetAddress Socket::getPeerAddr() const {
    struct sockaddr_in peeraddr;
    bzero(&peeraddr, sizeof peeraddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
    if(::getpeername(sockfd_, sockaddr_cast(&peeraddr), &addrlen) < 0)
    {
        perror("Socket::getLocalAddr");
    }
    return InetAddress(peeraddr);    
}

int Socket::read(void* buf, int len){
    return ::read(sockfd_, buf, len);
}

int Socket::write(const void* buf, int len){
    return ::write(sockfd_, buf, len);
}

Socket Socket::createTCP(){
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
    assert(sockfd >= 0);
    return Socket(sockfd);
}

Socket Socket::createUDP(){
    int sockfd = ::socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
    assert(sockfd >= 0);
    return Socket(sockfd);
}
