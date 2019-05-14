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

//����update������EventLoop�е�updateChannel����ʵ׷�����׻����ڵ���Epoll��updateChannel������
void Channel::update()
{
	addedToLoop_ = true;
	loop_->updateChannel(this);
}
//��updateͬ��������ȥ�������ڵ���Epoll��removeChannel����
void Channel::remove()
{
	assert(isNoneEvent());
	addedToLoop_ = false;
	loop_->removeChannel(this);
}

/*
handleEventWithGuard��
����reventֵ��������ص��¼��ص�������
man 2 poll �ɼ�ÿ���궨��POLLXXX�ĺ��塣
*/
void Channel::handleEvent()
{
	eventHandling_ = true;
	if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))  // �ļ�����������
	{

		//std::cout << "fd = " << fd_ << " Channel::handle_event() POLLHUP" << std::endl;

		if (closeCallback_) closeCallback_();
	}

	if (revents_ & (EPOLLERR)) // �ļ���������������
	{
		if (errorCallback_) errorCallback_();
	}
	if (revents_ & (EPOLLIN | EPOLLPRI))  // �ļ���������д
	{
		if (readCallback_) readCallback_();
	}
	if (revents_ & EPOLLOUT)                     // �ļ��������ɶ�
	{
		if (writeCallback_) writeCallback_();
	}
	eventHandling_ = false;

	//std::cout << "eventHandling_" << std::endl;
}



