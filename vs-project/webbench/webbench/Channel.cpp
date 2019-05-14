// @Author Senwang
// @Email senwang94@gmail.com


#include "Channel.h"
#include "EventLoop.h"
#include <iostream>
#include <assert.h>
#include <sys/epoll.h>


const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI| EPOLLET;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fd)
	: loop_(loop),
	fd_(fd),
	events_(0),
	revents_(0),
	index_(-1),
	eventHandling_(false),
	addedToLoop_(false)
{
}

Channel::~Channel()
{
	if (loop_->isInLoopThread())
	{
		// std::cout << "   ~Channel()    " << endl;
		assert(!loop_->hasChannel(this));
	}
}

void Channel::tie(const std::shared_ptr<void>& obj)
{
	tie_ = obj;
	tied_ = true;
}

//这里update调用了EventLoop中的updateChannel，其实追根究底还是在调用Epoll中updateChannel方法。
void Channel::update()
{
	addedToLoop_ = true;
	loop_->updateChannel(this);
}
//与update同理，层层调用去看还是在调用Epoll中removeChannel方法
void Channel::remove()
{
	assert(isNoneEvent());
	addedToLoop_ = false;
	loop_->removeChannel(this);
}

/*
handleEventWithGuard：
根据revent值来调用相关的事件回调函数。
man 2 poll 可见每个宏定义POLLXXX的含义。
*/
void Channel::handleEvent()
{
	eventHandling_ = true;
	if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))  // 文件描述符挂起
	{

		//std::cout << "fd = " << fd_ << " Channel::handle_event() POLLHUP" << std::endl;

		if (closeCallback_) closeCallback_();
	}

	if (revents_ & (EPOLLERR)) // 文件描述符发生错误
	{
		if (errorCallback_) errorCallback_();
	}
	if (revents_ & (EPOLLIN | EPOLLPRI))  // 文件描述符可写
	{
		if (readCallback_) readCallback_();
	}
	if (revents_ & EPOLLOUT)                     // 文件描述符可读
	{
		if (writeCallback_) writeCallback_();
	}
	eventHandling_ = false;

	//std::cout << "eventHandling_" << std::endl;
}



