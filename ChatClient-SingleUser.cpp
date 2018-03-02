///
/// 单用户版Chat客户端
/// 起两个线程,一个线程专门负责读,一个线程专门负责写
///

#include "./TCP/TcpClient.h"
#include "./TCP/InetAddress.h"
#include "./Base/Timestamp.h"
#include "./Net/EventLoop.h"
#include "./Net/EventLoopThreadPool.h"

#include "Codec.h"

#include <cstdio>
#include <functional>
#include <mutex>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <thread>
#include <chrono>
#include <string>
using namespace std::placeholders;

class ChatClient {
public:
    ChatClient(EventLoop* loop, const InetAddress& servAddr)
    : client_(loop, servAddr),
      conn_(client_.connection()),
      codec_(
          std::bind(&ChatClient::onStringMessage, this, _1, _2, _3)
      )  
    {
        client_.setConnectionCallback(
            std::bind(&ChatClient::onConnection, this, _1)
        );
        client_.setMessageCallback(
            std::bind(&Codec::onMessage, &codec_, _1, _2, _3)
        );
        client_.enableRetry();
    }

    void connect() {
        client_.connect();
    }
    void disconnect() {
        client_.disconnect();
    }

    void sendToServer(const std::string msg) {
        {
            std::lock_guard<std::mutex> locker(mutex_);
            printf("Client send [%s]\n", msg.c_str());
            if(conn_) { 
                codec_.send(conn_, conn_->localAddress().toIpPort()+" "+ msg);
            }
        }
    }

    //default dtor is okay


private:

    void onConnection(const TcpConnectionPtr& conn) {
        printf("%s->%s: connected? %d (0 for Down, 1 for UP)\n",
              conn->localAddress().toIpPort().c_str(),
              conn->peerAddress().toIpPort().c_str(),
              conn->connected());
        std::lock_guard<std::mutex> locker(mutex_);
        if(conn->connected()) {
            conn_ = conn;
        }
        else {
            conn_.reset();
        }
    }

    void onStringMessage(const TcpConnectionPtr& conn, const std::string msg,Timestamp recvTime) {
        // 接收到了消息,这个时候要做什么呢,很简单,打印出来就是了
        printf("\n");      
        printf("[%s]\t", recvTime.toFormattedString(false).c_str()); // 消息中没有格式
        printf(">>>  %s\n", msg.c_str());
        //std::cout << msg << "\n" << std::endl;
    }

    TcpClient client_;
    std::mutex mutex_; //保护conn_
    TcpConnectionPtr conn_;
    Codec codec_;
};



int main(int argc, char** argv) {
    if(argc != 3){
        printf("Usage: %s <chatServer IP> <Port>\n", argv[0]);
        exit(0);
    }
    EventLoopThread loopThread;  //这个进程, 析构函数包含了线程的回收
    InetAddress serverAddr(argv[1], atoi(argv[2]));
    ChatClient client(loopThread.startLoop(), serverAddr);
    client.connect(); //连接服务器--这里有一次runInLoop
    std::string line;
    
    //主线程接收键盘输入
    while(true){
        printf("\n$$: ");
        std::getline(std::cin, line);
        if(line.size() == 0) {
            printf("Empty messge is not allowed!");
            continue;
            
        }
        client.sendToServer(line);
    }

    client.disconnect();
    // using namespace std::chrono_literals;
    // printf("Exitting...\n");
    // std::this_thread::sleep_for(2s);
    // printf("Exitted!\n");


    return 0;
}