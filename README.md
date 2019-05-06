# An Efficient C++ Web Server

## Inroduction

本项目为使用C++11编写的Web服务器，实现了对GET、HEAD的解析，可以处理静态的资源，支持HTTP长链接，支持管线化请求，同时实现了双缓冲的异步日志，来实现对服务器运行状态的记录。

| Part Ⅰ | Part Ⅱ | Part Ⅲ |
|:------:|:------:|:------:|
| [并发模型](并发模型) | [连接维护](连接维护) | [测试与改进](测试与改进)  |


## Envoirment
- OS:Ubuntu 16.04（虚拟机）

- Complier: g++ 5.4.0

## Usage
- 我使用的是vs2017远程调试linux虚拟机，直接运行代码即可

## Technical Points
- 采用Reactor模式，使用Epoll边沿触发的IO多路复用技术 + 非阻塞IO
- 使用主线程 + 线程池的方式，线程池可以避免频繁的线程创建与销毁消耗
- 采用基于红黑树（STL map）+ Epoll的定时器实现连接超时关闭
- 主线程只负责accept请求，然后以Round Robin的方式分配给线程池中的IO线程，锁的争用只会出现在主线程与某一IO线程中。
- 使用eventfd实现了线程间的异步唤醒
- 使用双缓冲区实现了简单的异步日志系统
- 使用智能指针等RAII机制来减少内存泄漏的可能
- 使用状态机来解析HTTP请求，支持管线化请求
- 支持优雅的关闭连接

## Model
并发模型采用Reactor + 非阻塞IO + 线程池（event per thread），新请求Round Robin分配，详情介绍请参考[并发模型](并发模型)

![model](https://github.com/wangsen1994/web_server/blob/master/datum/model.png)

## EventLoop

![eventloop](https://github.com/wangsen1994/web_server/blob/master/datum/eventloop.png)

