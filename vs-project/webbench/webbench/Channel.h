// @Author Senwang
// @Email senwang94@gmail.com

#pragma once


#include "noncopy.h"

#include <functional>
#include <memory>


class EventLoop;
///
/// A selectable I/O channel.
///
/// This class doesn't own the file descriptor.
/// The file descriptor could be a socket,
/// an eventfd, a timerfd, or a signalfd
class Channel :noncopy
{
public:
	typedef std::function<void()> EventCallback;

	Channel(EventLoop* loop, int fd);
	~Channel();

	void setReadCallback(EventCallback cb)
	{
		readCallback_ = std::move(cb);
	}
	void setWriteCallback(EventCallback cb)
	{
		writeCallback_ = std::move(cb);
	}
	void setCloseCallback(EventCallback cb)
	{
		closeCallback_ = std::move(cb);
	}
	void setErrorCallback(EventCallback cb)
	{
		errorCallback_ = std::move(cb);
	}

	void tie(const std::shared_ptr<void>&);

	int fd() const { return fd_; }
	int events() const { return events_; }
	void set_revents(int revt) { revents_ = revt; } // used by pollers
	// int revents() const { return revents_; }
	bool isNoneEvent() const { return events_ == kNoneEvent; }

	//update������ʵ�������ᷢ����ʵ���ǵ�����EPollPoller���updateChannel������ע����޸�
	/* ������channel���������ö��¼��������ûص�������Ȼ��enableReading����������ͷǳ��ؼ��ˣ�����������events������
	�¼����ͣ�Ȼ��update�����update���ø��¼�����loop������loop�ϵ�EPollPoller��updateChannel */

	void enableReading() { events_ |= kReadEvent; update(); }
	void disableReading() { events_ &= ~kReadEvent; update(); }
	void enableWriting() { events_ |= kWriteEvent; update(); }
	void disableWriting() { events_ &= ~kWriteEvent; update(); }
	void disableAll() { events_ = kNoneEvent; update(); }
	bool isWriting() const { return events_ & kWriteEvent; }
	bool isReading() const { return events_ & kReadEvent; }

	// for Poller
	int index() { return index_; }
	void set_index(int idx) { index_ = idx; }


	EventLoop* ownerLoop() { return loop_; }
	void remove();

	void handleEvent();

private:

	void update();

	static const int kNoneEvent;
	static const int kReadEvent;
	static const int kWriteEvent;

	EventLoop* loop_;
	const int  fd_;
	int        events_;
	int        revents_; // it's the received event types of epoll or poll
	int        index_; // ����epoll�еĸ�������;


	std::weak_ptr<void> tie_;
	bool tied_;
	bool eventHandling_;    //�Ƿ�����ִ�лص�
	bool addedToLoop_;      //�Ƿ���뵽ʱ��ѭ��   

	EventCallback readCallback_;
	EventCallback writeCallback_;
	EventCallback closeCallback_;
	EventCallback errorCallback_;

};


