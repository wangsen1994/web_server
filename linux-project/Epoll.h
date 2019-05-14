// @Author Senwang
// @Email senwang94@gmail.com

#pragma once
#include "base/noncopy.h"
#include "Channel.h"
#include "EventLoop.h"
#include <vector>
#include <sys/epoll.h>
#include <map>

struct epoll_event;

class Epoll :noncopy
{
public:
	typedef std::vector<struct epoll_event> EventList;
	typedef std::vector<Channel*> ChannelList;
	typedef std::map<int, Channel*> ChannelMap;
public:
	Epoll(EventLoop* loop);
	~Epoll();

	void poll(int timeoutMs, ChannelList* activeChannels);
	void updateChannel(Channel* channel);
	void removeChannel(Channel* channel);
	bool hasChannel(Channel* channel) const;
private:

	void update(int operation, Channel* channel);
	void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
	void assertInLoopThread() const
	{
		ownerLoop_->assertInLoopThread();
	}
	int epollfd_;
	EventLoop* ownerLoop_;
	EventList events_;
	ChannelMap channels_;
	static const int kInitEventListSize = 16;
};

