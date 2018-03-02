#ifndef CHATSERVER_CODEC_H
#define CHATSERVER_CODEC_H

#include "./Base/Common.h"
#include "./Net/Buffer.h"
#include "./Net/Callback.h"
#include "./TCP/TcpConnection.h"
#include "./TCP/SocketOps.h" //sockets::hostToNetwork32()
#include "./Base/Timestamp.h"


#include <string>
#include <functional>
#include <iostream>
#include <stdio.h>
#include <memory>

class Codec : smuduo::noncopyable {
public:
    typedef std::function<void (const TcpConnectionPtr&,
                           const std::string&  message,
                           Timestamp)> StringMessageCallback;

    Codec(StringMessageCallback cb)
        : stringMessageCallback_(cb)
    {
        
    }
    // default dtor, cpCtor, mvCtor is okay

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp recvTime){ //来了Buffer消息,现在需要解码,这就相当于是decode        
        while(buf->readableBytes() >= kHeaderLen){ // 表示长度的消息已经够了
            //const int32_t len = buf->peekInt32(); //这里不能用readInt,因为消息长度可能不够
            const void* data = buf->peek();
            int32_t be32 = *static_cast<const int32_t*>(data); //SIGBUS
            const int32_t len = sockets::networkToHost32(be32);
            if(len > kMaxMessageLength || len < 0) {
                // 需要通知用户吗? 还是在服务器端进行校验?
                printf("LOG_ERROR: Invalid message length: %d\n", len);
                conn->shutdown();
                break;
            }
            else if(buf->readableBytes() >= len + kHeaderLen) {
                // 现在length合法, 缓冲区也有足够的数据可读,现在要做的就是将其提取出来
                buf->retrieve(kHeaderLen); //取出消息头的4个字节
                std::string msg(buf->peek(), len);
                printf("\n");
                stringMessageCallback_(conn, msg, recvTime); //解码之后的string交给ChatServer处理
                buf->retrieve(len);
            }
            else{ //消息还没有到全,等一会再读
                break;
            }
            
            
        }
    }


    void send(const TcpConnectionPtr& conn, const std::string msg) { //将解码后的msg重新编码,再发送给conn这个连接
        //int32_t be32 = sockets::hostToNetwork32(msgLen);
        //Buffer buf;
        std::shared_ptr<Buffer> buf = std::make_shared<Buffer>();
        //buf.prependInt32(msgLen);
        buf->append(msg);
        int32_t msgLen = static_cast<int32_t>(msg.size()); //消息头部的长度不包括自己的4个字节
        int32_t be32 = sockets::hostToNetwork32(msgLen);

        buf->prepend(&be32, sizeof be32);
       // std::string str = buf.retrieveAllAsString_withPrepend();
      //  printf("Codec::send %d bytes, first char = %c\n", buf.peekInt32(), *(buf.peek()+4));    
        //conn->send(&buf);
       // conn->send(buf.peek(), buf.readableBytes());
       conn->send(buf);
    }
private:
    StringMessageCallback stringMessageCallback_;
    const static size_t kHeaderLen = sizeof(int32_t);
    const static int kMaxMessageLength = 65536; //单条消息的最大长度
};




#endif