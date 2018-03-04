#include "./Net/EventLoop.h"
#include "./TCP/TcpServer.h"
#include "./TCP/InetAddress.h"
#include "./Net/EventLoopThreadPool.h"
#include "./Net/Callback.h"

#include "Codec.h" //消息的编解码器 --- 最简单的格式, 头部4个字节代表消息长度


#include <map>
#include <set>
#include <memory>
#include <functional>
#include <assert.h>


using namespace std::placeholders;

class ChatServer : smuduo::noncopyable {
public:
    //在构造的时候必须提供两个回调: onConnection, onMessage
    ChatServer(EventLoop* loop, const InetAddress& listenAddr)
        : loop_(loop),
          server_(loop, listenAddr),
          codec_(
              std::bind(&ChatServer::onStringMessage, this, _1, _2, _3)
          )
    {
        server_.setConnectionCallback(
            std::bind(&ChatServer::onConnection, this, _1)
        );

        server_.setMessageCallback(
           std::bind(&Codec::onMessage, &codec_, _1, _2, _3) //ConnPtr Buffer* Timestamp
       );
    }
    
    // default dtor is okay


    void start() //开始工作
    {
        server_.start();
    }

private:
    EventLoop* loop_;
    TcpServer server_;
    std::set<TcpConnectionPtr> conns_; 
    Codec codec_;
    
    void onConnection(const TcpConnectionPtr& conn){
        if(conn->connected()) { 
            conns_.insert(conn);
          //  codec_.send(conn, "WelCome to smuduoChat\n");
        }
        else { //已经关闭连接,此时连接断开了
            conns_.erase(conn);
        }
    }

    void onStringMessage(const TcpConnectionPtr& conn, const std::string msg, Timestamp recvTime){
        assert(conns_.find(conn) != conns_.end()); //确保发消息来的连接是server已经建立的连接
        //群发消息 -- 之后可以根据消息类型选取不同的用户进行发送
        //printf("Server sent [%s]\n", msg.c_str());
        for(const TcpConnectionPtr& conn : conns_) {
            codec_.send(conn, msg);
        }
    }

//    std::unique_ptr<EventLoopThreadPool>  threadPool_; //线程池
    
   
};


// ChatServer 0.1.0 只具有消息转发功能

int main(int argc, char** argv) {
    printf("============================================\n\n");
    printf("       ChatServer 0.1.0 - By Free50u1       \n\n");
    printf("============================================\n\n\n");

    // argv[1] - port: 缺省为9981

    EventLoop loop; //主线程
    int port = (argc > 1) ? atoi(argv[1]) : 9981;
    InetAddress listenAddr(port);

    ChatServer server(&loop, listenAddr);
    server.start();

    loop.loop();

    return 0;
}   