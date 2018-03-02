#ifndef SMUDUO_NET_CALLBACK_H
#define SMUDUO_NET_CALLBACK_H

#include <functional>
#include <memory>
#include "../Base/Timestamp.h"

namespace smuduo{

template <typename To, typename From>
inline std::shared_ptr<To> down_pointer_cast(const std::shared_ptr<From>& f) {
    if(false) { //???
        implicit_cast<From*, To*>(0);
    }
#ifndef NDEBUG
    assert(f == NULL || dynamic_cast<To *>(get_pointer(f)) != NULL);
#endif
    return std::static_pointer_cast<To>(f);
}
}

using namespace smuduo;

class Buffer;
class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void()> TimerCallback;
typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void (const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;

typedef std::function<void (const TcpConnectionPtr&, Buffer*, Timestamp)> 
                            MessageCallback;
//typedef std::function<void (const TcpConnectionPtr&, char* buf, ssize_t len)> 
//                            MessageCallback;


void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn, 
                            Buffer* buffer,
                            Timestamp recvTime);

#endif