#ifndef SMUDUO_TCP_SOCKET_H
#define SMUDUO_TCP_SOCKET_H 

#include "../Base/Common.h"
#include "InetAddress.h"
#include <utility> //swap

class InetAddress;

using namespace smuduo;

// RAII handle for socket fd
class Socket : noncopyable {
public:
    explicit Socket(int sockfd);
    ~Socket(); //Close sockfd_

    Socket(Socket&& rhs)
      : Socket(rhs.sockfd_)
    {
        rhs.sockfd_ = -1;
    }

    Socket& operator=(Socket&& rhs){
        swap(rhs);
        return *this;
    }

    void swap(Socket& rhs){
        std::swap(sockfd_, rhs.sockfd_);
    }

    int fd() const {return sockfd_;}

    //Sockets API
    void bindOrDie(const InetAddress& addr);
    void listenOrDie();
    /// On success, returns a non-negative integer that is
    /// a descriptor for the accepted socket, which has been
    /// set to non-blocking and close-on-exec. *peeraddr is assigned.
    /// On error, -1 is returned, and *peeraddr is untouched.
    int accept(InetAddress* peeraddr);

    //return 0 on success
    int connect(const InetAddress& addr);
    void shutdownWrite();

    void setReuseAddr(bool on);
    void setTcpNoDelay(bool on);

    InetAddress getLocalAddr() const;
    InetAddress getPeerAddr() const;

    int read(void* buf, int len);
    int write(const void* buf, int len);

    //factory methods
    static Socket createTCP();
    static Socket createUDP();

 

private:  
    int sockfd_;
};

#endif