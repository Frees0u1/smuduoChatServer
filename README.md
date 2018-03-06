# 基于SMUDUO网络库的高并发在线聊天服务器

## 由来
阅读陈硕先生《Linux多线程服务端编程》后，受益匪浅。匆匆浏览，深知纸上得来终觉浅，于是对照书中代码，一步步重新实现Muduo网络库。因功能相较于MUDUO少很多，故命名为SMUDUO, S即为small.

除根据陈硕先生在书中以及博览网《网络编程实战》的指导外，smuduo大量引入C++11标准内容，替代书中许多boost依赖以及自实现的基本库函数，比如thread,mutex,lock_guard,condition_variable, bind, function，atomic等等。


## 文件介绍

### 网络库代码部分
* ./Base: 基础代码，如CurrentThread Timestamp等
* ./Net: 网络相关代码，如重要的Reactor三剑客EventLoop/Channel/Poller
* ./TCP: 处理TCP连接的先关代码

### 聊天室程序文件代码
* ChatServer.cpp -- 服务端实现
* ChatClient.cpp -- 客户端实现,起两个线程, EventLoop线程负责网络IO, 主线程负责读取键盘输入
* Codec.h        -- 简单的编解码器,消息格式只是简单在消息头部插入一个int32_t, 表示消息长度

### 其他
* ./CppTest: C++语言特性的测试代码,测试包括了lock_guard/furture/condition_variable等
* ./Note.md 学习笔记


## 编译与运行
```make server``` 编译服务器可执行文件

```make client``` 编译客户端可执行文件

```make all``` 编译两者

### 运行
* ```./chatserver <port>``` 其中port缺省为9981
* ```./chatclient <IP> <port>``` 不可缺省
* 因为客户端有连接失败自动重连的功能,所以两者启动顺序无强制要求


### 初步测试
测试程序主要用```ChatClient-MultiClient.cpp```文件,多用户客户端在一个线程池中运行指定数目的Chatclient,每个客户端随机间隔(0.5~1s)向服务器发送定长的消息,统计观察每个客户端发送和接收到的消息数目来判断功能是否正常. 在虚拟机单机测试,用户数目设为200的条件下,测试数据如下:
```
====================================

    多用户客户端测试报告
 总计发送16135条,共2065280字节
 总计接收3227000条,共413056000字节


====================================
```
发送与接收数据匹配,基本功能能够保证.

**Webbench压力测试**
用Smuduo网络库编写了HTTP200Server, 即不管客户端发送任何请求, 均只回复HTTP200响应报文. 在此条件下,用webbench做了简单的压力测试,可支持1w+并发连接
```
Webbench - Simple Web Benchmark 1.5
Copyright (c) Radim Kolar 1997-2004, GPL Open Source Software.

Benchmarking: GET http://localhost:9981/
10240 clients, running 10 sec.

Speed=2794656 pages/min, 4983321 bytes/sec.
Requests: 465776 susceed, 0 failed.
```


## TODO

* 网络库中添加高性能logger
* 加入登录验证,私发消息等功能


