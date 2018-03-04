NET_SRC = ./Net/EventLoop.cpp ./Net/Channel.cpp ./Net/TimerQueue.cpp ./Net/Timer.cpp ./Net/EventLoopThread.cpp ./Net/EPoller.cpp ./Net/Buffer.cpp ./Net/EventLoopThreadPool.cpp
BASE_SRC = ./Base/CurrentThread.cpp ./Base/Timestamp.cpp
TCP_SRC = ./TCP/Acceptor.cpp ./TCP/InetAddress.cpp ./TCP/Socket.cpp ./TCP/SocketOps.cpp ./TCP/TcpServer.cpp ./TCP/TcpConnection.cpp ./TCP/Connector.cpp ./TCP/TcpClient.cpp
CC = g++
CXXFLAGS = -O0 -std=c++17 -g -pthread

NET_OBJ = ./Net/EventLoop.o ./Net/Channel.o ./Net/TimerQueue.o ./Net/Timer.o ./Net/EventLoopThread.o ./Net/EPoller.o ./Net/Buffer.o ./Net/EventLoopThreadPool.o
BASE_OBJ = ./Base/CurrentThread.o ./Base/Timestamp.o
TCP_OBJ = ./TCP/Acceptor.o ./TCP/InetAddress.o ./TCP/Socket.o ./TCP/SocketOps.o ./TCP/TcpServer.o ./TCP/TcpConnection.o ./TCP/Connector.o ./TCP/TcpClient.o


all:
	$(CC) $(NET_SRC) $(BASE_SRC) $(TCP_SRC) ChatServer.cpp -o chatserver $(CXXFLAGS)
	$(CC) $(NET_SRC) $(BASE_SRC) $(TCP_SRC) ChatClient-SingleUser.cpp -o chatclient $(CXXFLAGS)
server:
	$(CC) $(NET_SRC) $(BASE_SRC) $(TCP_SRC) ChatServer.cpp -o chatserver $(CXXFLAGS)
client:
	$(CC) $(NET_SRC) $(BASE_SRC) $(TCP_SRC) ChatClient-SingleUser.cpp -o chatclient $(CXXFLAGS)

multiclient:
	$(CC) $(NET_SRC) $(BASE_SRC) $(TCP_SRC) ChatClient-MultiUser.cpp -o chatmulitclient $(CXXFLAGS)