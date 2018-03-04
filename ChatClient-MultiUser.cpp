//
// 同时包含多个用户的聊天客户端,主要用于测试,因此每个客户端发送的消息是一个指定长度的消息(默认128)(不需要从键盘读入)
// 用户放在一个线程池中,数目由用户指定.(并不是one user per thread, 而是one loop per thread, 每个loop可以有多个)
// 线程池数目由用户指定,默认为1,最好不要超过电脑的核数
//

#include "./Net/EventLoop.h"
#include "./Net/EventLoopThreadPool.h"
#include "Codec.h"
#include "./TCP/TcpClient.h"
#include "./TCP/InetAddress.h"
#include "./Base/Timestamp.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <assert.h>
#include <string>
#include <functional>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <thread>
#include <chrono>
#include <string>
#include <mutex>

int g_usrNum;
int g_threadNum;
int g_msgLen;
std::string g_msgContent;

using namespace std::placeholders;



class ChatClient  : smuduo::noncopyable {
public:
    ChatClient(EventLoop* loop, const InetAddress& servAddr)
    : client_(loop, servAddr),
      conn_(client_.connection()),
      codec_(
          std::bind(&ChatClient::onStringMessage, this, _1, _2, _3)
      ),
      recvMsgCnt_(0),
      recvMsgBytes_(0),
      sendMsgCnt_(0),
      sendMsgBytes_(0)
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

    int RecvMsgCnt() const {
        return recvMsgCnt_;
    }
    long long RecvMsgBytes() const {
        return recvMsgBytes_;
    }
    int SendMsgCnt() const {
        return sendMsgCnt_;
    }
    long long SendMsgBytes() const {
        return sendMsgBytes_;
    }

    void sendToServer(const std::string msg) {
        {
            std::lock_guard<std::mutex> locker(mutex_);
        //    printf("Client send [%s]\n", msg.c_str());
            if(conn_) { 
                codec_.send(conn_, msg);
            }
            sendMsgCnt_++;
            sendMsgBytes_ += msg.size();
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
        // 接收到了消息,这个时候要做什么呢,很简单,统计并忽略之
        recvMsgCnt_ ++;
        recvMsgBytes_ += msg.size();
    }

    TcpClient client_;
    std::mutex mutex_; //保护conn_
    TcpConnectionPtr conn_;
    Codec codec_;

    int recvMsgCnt_; //收到的消息条数
    long long recvMsgBytes_; //收到的消息字符数
    int sendMsgCnt_;
    int sendMsgBytes_;
};



class ChatMultiClient : smuduo::noncopyable{
public:
    ChatMultiClient(EventLoop* loop, InetAddress serverAddr, int userNum)
        : baseloop_(loop),
          serverAddr_(serverAddr),
          userNum_(userNum),
          threadPool_(new EventLoopThreadPool(loop)),
          gen_(rd_()),
          dis_(0.5, 1.0)
    {
       
    }
    ~ChatMultiClient(){

    }

    void start(){
         // 创建并启动线程池
        threadPool_->setThreadNum(g_threadNum); 
        threadPool_->start();
        for(int i = 0; i < userNum_; i++) {
            EventLoop* ioLoop  = threadPool_->getNextLoop();
            chatclients_.push_back(std::make_shared<ChatClient>(ioLoop, serverAddr_)); 
            chatclients_.back()->connect();
            TimerId timerId = ioLoop->runEvery(
               dis_(gen_), //随机的时间
                std::bind(&ChatClient::sendToServer, chatclients_.back(), g_msgContent)
            );
            ioLoop->runAfter(
                60, // 1分钟后结束发送
                std::bind(&EventLoop::cancel, ioLoop, timerId)
            );
        }
    }

    void stop(){ //需确保所有客户端的发送任务都已完成
        int idx = 0;
        int totalSendCnt = 0, totalRecvCnt = 0;
        long long totalSendBytes = 0, totalRecvBytes = 0;
        for(auto& client : chatclients_){
            printf("客户端%d: 发送%d条(%lld Bytes), 接收%d条(%lld Bytes)\n",
                    idx++, client->SendMsgCnt(), client->SendMsgBytes(),
                    client->RecvMsgCnt(), client->RecvMsgBytes());
            totalSendCnt += client->SendMsgCnt();
            totalSendBytes += client->SendMsgBytes();
            totalRecvCnt += client->RecvMsgCnt();
            totalRecvBytes += client->RecvMsgBytes();

            client->disconnect();
        }
        
        printf("\n\n====================================\n\n");
        printf("             多用户客户端测试报告           \n");
        printf(" 总计发送%d条,共%lld字节\n", totalSendCnt, totalSendBytes);
        printf(" 总计接收%d条,共%lld字节\n",totalRecvCnt, totalRecvBytes);
        printf("\n\n====================================\n\n");

    }

    void setThreadNum(int threadNum){
        assert(0 <=threadNum);
        threadPool_->setThreadNum(threadNum);
    }


private:
    EventLoop* baseloop_;
    InetAddress serverAddr_;
    int userNum_;
    std::unique_ptr<EventLoopThreadPool>  threadPool_;
//    std::vector<ChatClient> chatclients_; //所有的客户端
    std::vector<std::shared_ptr<ChatClient>> chatclients_;

    //随机数
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_real_distribution<> dis_;
};




int main(int argc, char* argv[]){
    if(argc < 4){
        printf("Usage: <%s> <Server IP> <Port> <userNum> <threadNum = 1> <msgLen = 128>", argv[0]);
        exit(-1);
    }
    g_usrNum = atoi(argv[3]);
    g_threadNum = (argc >= 5) ? atoi(argv[4]) : 1;
    g_msgLen = (argc >= 6) ? atoi(argv[5]) : 128;
    g_msgContent = std::string(g_msgLen, 'S');

    EventLoop loop;
    InetAddress serverAddr(argv[1], atoi(argv[2]));

    ChatMultiClient multiClient(&loop, serverAddr, g_usrNum);
    multiClient.start();

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(2s*50);
    multiClient.stop();

    return 0;
}