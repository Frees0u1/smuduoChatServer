#ifndef SMUDUO_TCP_SOCKETOPS_H
#define SMUDUO_TCP_SOCKETOPS_H

#include <arpa/inet.h>
#include <endian.h>

namespace smuduo {
namespace sockets{

inline uint64_t hostToNetwork64(uint64_t host64){ 
    return htobe64(host64);
}

inline uint32_t hostToNetwork32(uint32_t host32){ 
    return htonl(host32);
}
inline uint16_t hostToNetwork16(uint16_t host16){ 
    return htons(host16);
}
// 8bytes is the same between HostEndian and NetEndian

inline uint64_t networkToHost64(uint64_t net64) {
    return be64toh(net64);
}
inline uint32_t networkToHost32(uint32_t net32) {
    return ntohl(net32);
}
inline uint16_t networkToHost16(uint16_t net16) {
    return ntohs(net16);
}

///
/// Creates a non-blocking socket file descriptor
/// abort immediatily if any error

int createNonBlockingOrDie();

int  connect(int sockfd, const struct sockaddr_in& addr);
void bindOrDie(int sockfd, const struct sockaddr_in& addr);
void listenOrDie(int sockfd);
int accept(int sockfd, struct sockaddr_in* addr);
void close(int sockfd);

ssize_t read(int sockfd, void *buf, size_t count);
ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);
ssize_t write(int sockfd, const void *buf, size_t count);

void toHostPort(char* buf, size_t size, const struct sockaddr_in& addr);
void fromHostPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
struct sockaddr_in getLocalAddr(int sockfd);
struct sockaddr_in getPeerAddr(int sockfd);
int getSocketError(int sockfd);
bool isSelfConnect(int sockfd);
} //end of namespace of sockets
} // end of namespace of smuduo



#endif