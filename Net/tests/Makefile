NET_SRC = ../EventLoop.cpp ../Channel.cpp ../Poller.cpp ../TimerQueue.cpp ../Timer.cpp ../EventLoopThread.cpp
BASE_SRC = ../../Base/CurrentThread.cpp ../../Base/Timestamp.cpp
CC = g++
CXXFLAGS = -O0 -std=c++14 -g -pthread

testThreadSafe:
	$(CC) $(NET_SRC) $(BASE_SRC) ./testThreadSafeOfTimerQueue.cpp -o ./testTQ_TS $(CXXFLAGS)

testEventLoopThread:
	$(CC) $(NET_SRC) $(BASE_SRC) ./testEventLoopThread.cpp $(CXXFLAGS)
test4:
	$(CC) $(NET_SRC) $(BASE_SRC) ./test4.cpp $(CXXFLAGS)