//
// 创建N个连接
//

#include "../Connector.h"
#include "../../Net/EventLoop.h"
#include "../InetAddress.h"

#include <vector>
#include <memory>
#include <map>

EventLoop* g_loop;

//int connCnt; //连接次数
using namespace std;

int connCnt = 0;
bool allDone = false;
int N;

void connectCallback(int sockfd) {
    printf("connected.\n");
    if(++connCnt == N) { 
        g_loop->quit(); //所有连接器都连接成功后退出循环
    }
}

int main(int argc, char** argv) {
    EventLoop loop;
    g_loop = &loop;
    InetAddress addr("127.0.0.1", 9981);
   

    N = (argc == 2) ? atoi(argv[1]) : 1; //连接数量,默认为1
    vector<ConnectorPtr> connectors;
    for(int i = 0; i < N; i++) {
        ConnectorPtr tmp = make_shared<Connector>(&loop, addr);
        tmp->setNewConnectionCallback(connectCallback);
        tmp->start();
        connectors.push_back(tmp);
    }

    loop.loop();
}