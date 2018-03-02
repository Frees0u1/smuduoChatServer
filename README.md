# 基于SMUDUO的网络库的高并发在线聊天服务器

## 由来
阅读陈硕先生《Linux多线程服务端编程》后，受益匪浅。匆匆浏览，深知纸上得来终觉浅，于是对照书中代码，一步步重新实现Muduo网络库。因功能相较于陈硕先生少很多，故命名为SMUDUO, S即为small.

除根据陈硕先生在书中以及博览网《网络编程实战》的指导外，smuduo大量引入C++11标准内容，替代书中许多boost依赖以及自实现的基本库函数，比如thread,mutex,lock_guard,condition_variable, bind, function，atomic等等。


## 文件介绍

### 网络库代码部分
* ./Base: 基础代码，如CurrentThread Timestamp等
* ./Net: 网络相关代码，如重要的Reactor三剑客EventLoop/Channel/Poller
* ./TCP: 处理TCP连接的先关代码

### 聊天室程序文件代码
* ChatServer.cpp

### 其他代码
* ./CppTest: C++语言特性的测试代码,测试包括了lock_guard/furture等


