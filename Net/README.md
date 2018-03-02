# 基本网络库代码

* **Buffer.h & Buffer.cpp** 网络库底层的Buffer类的定义及实现
* 多线程IO复用三大类
    * EventLoop: 控制poller，每个线程至多一个loop
    * Poller: 控制不同的Channel
    * Channel: 每个控制一个具体的fd

## EventLoop


## Channel


## Poller
