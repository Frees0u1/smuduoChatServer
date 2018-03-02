#include "./SocketAPI/Acceptor.h"
#include "./SocketAPI/TcpStream.h"
#include "./SocketAPI/Socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>

int main(int argc, char **argv){
    if(argc != 2){
        printf("Usage: %s <port>\n", argv[0]);        
    }
    InetAddress listenAddr(atoi(argv[1]));
    Acceptor acceptor(listenAddr);
    int listenfd = acceptor.fd();
    int connfd;
    struct epoll_event *ep_events;
    struct epoll_event event;
    int epfd, event_cnt;
    

    epfd = epoll_create(EPOLL_SIZE);
    ep_events = new struct epoll_event[EPOLL_SIZE];

    event.events = EPOLLIN; //监视是否有可读数据
    event.data.fd = listenfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event);

    while(1){
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if(event_cnt == -1){
            puts("epoll_wait() error!\n");
            break;
        }
        for(int i = 0; i < event_cnt; i++){
            if(ep_events[i].data.fd == listenfd){ //有新用户加入连接
                
            }
            else {
                
            }
            
        }

    }

}