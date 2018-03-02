#include "SocketOps.h"
#include "../Base/Common.h" // implicit_cast

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>  //snprintf
#include <strings.h> // bzero
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>

using namespace smuduo;

namespace {

typedef struct sockaddr SA;

const SA* sockaddr_cast(const struct sockaddr_in* addr) {
    return static_cast<const SA*>(implicit_cast<const void*>(addr));
}

SA* sockaddr_cast(struct sockaddr_in* addr) {
    return static_cast<SA *>(implicit_cast<void*>(addr));
}

void setNonBlockAndCloseOnExec(int sockfd) {
    // non-block
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = ::fcntl(sockfd, F_SETFL, flags);

    //close-on-exec
    flags = ::fcntl(sockfd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ret = ::fcntl(sockfd, F_SETFD, flags);
}


} // End of namespace 

int sockets::createNonBlockingOrDie() {
    //socket
    int sockfd = ::socket(AF_INET, 
                          SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                          IPPROTO_TCP);
    if(sockfd < 0) {
        printf("Error in createNonBlockingOrDie()\n");
        exit(-1);
    }
    return sockfd;
}

// 只是对connect的简单封装
int sockets::connect(int sockfd, const struct sockaddr_in& addr)
{
  return ::connect(sockfd, sockaddr_cast(&addr), sizeof addr);
}


void sockets::bindOrDie(int sockfd, const struct sockaddr_in& addr) {
    int ret = ::bind(sockfd, sockaddr_cast(&addr), sizeof addr);
    if(ret < 0) {
        printf("Error in bindOrDie()");
        exit(-1);
    }
}

int sockets::accept(int sockfd, struct sockaddr_in* addr) {
    socklen_t addrlen = sizeof *addr;

#if VARGRIND
    int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
    setNonBlockAndCloseOnExec(connfd);
#else
    int connfd = ::accept4(sockfd, sockaddr_cast(addr),
                            &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif 
    if(connfd < 0) {
        int savedErrno = errno;
        printf("LOG_SYSERR: sockets::accept");

        switch(savedErrno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO:  // ?????
            case EPERM: 
            case EMFILE: // per-process limit of open fd ??
                // expected errors
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                printf("unexpected error of accept: %d", savedErrno);
                exit(-1);
                break;
            default:
                printf("unknown error of accept: %d", savedErrno);
                exit(-1);
                break;   
        }
    }
    return connfd;
}

void sockets::close(int sockfd) {
    if(::close(sockfd) < 0) {
        printf("LOG_SYSERR: socket::close\n");
    }
}

ssize_t sockets::read(int sockfd, void *buf, size_t count)
{
  return ::read(sockfd, buf, count);
}

ssize_t sockets::readv(int sockfd, const struct iovec *iov, int iovcnt)
{
  return ::readv(sockfd, iov, iovcnt);
}

ssize_t sockets::write(int sockfd, const void *buf, size_t count)
{
  return ::write(sockfd, buf, count);
}


void sockets::toHostPort(char* buf, size_t size,
                         const struct sockaddr_in& addr)
{
  char host[INET_ADDRSTRLEN] = "INVALID";
  ::inet_ntop(AF_INET, &addr.sin_addr, host, sizeof host);
  uint16_t port = sockets::networkToHost16(addr.sin_port);
  snprintf(buf, size, "%s:%u", host, port);
}

void sockets::fromHostPort(const char* ip, uint16_t port,
                           struct sockaddr_in* addr)
{
  addr->sin_family = AF_INET;
  addr->sin_port = hostToNetwork16(port);
  if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
  {
    printf("LOG_SYSERR: sockets::fromHostPort\n");
  }
}

struct sockaddr_in sockets::getLocalAddr(int sockfd)
{
    struct sockaddr_in localaddr;
    bzero(&localaddr, sizeof localaddr);
    socklen_t addrlen = sizeof(localaddr);
    if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0) {
    printf("LOG_SYSERR: sockets::getLocalAddr\n");
    }
    return localaddr;
}

struct sockaddr_in sockets::getPeerAddr(int sockfd)
{
  struct sockaddr_in peeraddr;
  bzero(&peeraddr, sizeof peeraddr);
  socklen_t addrlen = sizeof(peeraddr);
  if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
  {
    printf("LOG_SYSERR: sockets::getPeerAddr\n");
  }
  return peeraddr;
}

int sockets::getSocketError(int sockfd)
{
  int optval;
  socklen_t optlen = sizeof optval;

  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
  {
    return errno;
  }
  else
  {
    return optval;
  }
}

bool sockets::isSelfConnect(int sockfd)
{
  struct sockaddr_in localaddr = getLocalAddr(sockfd);
  struct sockaddr_in peeraddr = getPeerAddr(sockfd);
  return localaddr.sin_port == peeraddr.sin_port
      && localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;
}