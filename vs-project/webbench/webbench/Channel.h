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

	//update函数其实层层深入会发现其实就是调用了EPollPoller类的updateChannel方法。注册或修改
	/* 创建个channel，比如设置读事件，先设置回调函数，然后enableReading（），这里就非常关键了，它会先设置events变量的
	事件类型，然后update，这个update会让该事件所在loop来调用loop上的EPollPoller的updateChannel */

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
	int        index_; // 用于epoll中的更新类型;


	std::weak_ptr<void> tie_;
	bool tied_;
	bool eventHandling_;    //是否正在执行回调
	bool addedToLoop_;      //是否加入到时间循环   

	EventCallback readCallback_;
	EventCallback writeCallback_;
	EventCallback closeCallback_;
	EventCallback errorCallback_;

};


