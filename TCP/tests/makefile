NET_SRC = ../../Net/EventLoop.cpp ../../Net/Channel.cpp ../../Net/TimerQueue.cpp ../../Net/Timer.cpp ../../Net/EventLoopThread.cpp ../../Net/EPoller.cpp ../../Net/Buffer.cpp ../../Net/EventLoopThreadPool.cpp
BASE_SRC = ../../Base/CurrentThread.cpp ../../Base/Timestamp.cpp
TCP_SRC = ../Acceptor.cpp ../InetAddress.cpp ../Socket.cpp ../SocketOps.cpp ../TcpServer.cpp ../TcpConnection.cpp ../Connector.cpp ../TcpClient.cpp
CC = g++
CXXFLAGS = -O0 -std=c++17 -g -pthread

test1:
	$(CC) $(NET_SRC) $(BASE_SRC) $(TCP_SRC) $(CXXFLAGS) test1_Acceptor.cpp -o test01

test7:
	$(CC) $(NET_SRC) $(BASE_SRC) $(TCP_SRC) $(CXXFLAGS) test7.cpp -o test07

test8:
	$(CC) $(NET_SRC) $(BASE_SRC) $(TCP_SRC) $(CXXFLAGS) test8.cpp -o test08

test3:
	$(CC) $(NET_SRC) $(BASE_SRC) $(TCP_SRC) $(CXXFLAGS) test3.cpp -o test03

test9:
	$(CC) $(NET_SRC) $(BASE_SRC) $(TCP_SRC) $(CXXFLAGS) test9.cpp -o test09

test10:
	$(CC) $(NET_SRC) $(BASE_SRC) $(TCP_SRC) $(CXXFLAGS) test10.cpp -o test010
test11:
	$(CC) $(NET_SRC) $(BASE_SRC) $(TCP_SRC) $(CXXFLAGS) test11.cpp -o test011

test12:
	$(CC) $(NET_SRC) $(BASE_SRC) $(TCP_SRC) $(CXXFLAGS) test12-N.cpp -o test012-C

test13:
	$(CC) $(NET_SRC) $(BASE_SRC) $(TCP_SRC) $(CXXFLAGS) test13-Client.cpp -o echoClient