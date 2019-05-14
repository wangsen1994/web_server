// @Author Senwang
// @Email senwang94@gmail.com

#include "Epoll.h"
#include "base/CurrentThread.h"
#include <iostream>
#include <unistd.h>
#include <assert.h>
#include <string.h>

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

Epoll::Epoll(EventLoop* loop)
	:ownerLoop_(loop),
	events_(kInitEventListSize),
	epollfd_(::epoll_create(EPOLL_CLOEXEC))
{
	//if (epollfd_ < 0)
		//std::cout << "epoll create error" << std::endl;
}
Epoll::~Epoll()
{
	::close(epollfd_);
}
void Epoll::poll(int timeoutMs, ChannelList* activeChannels)
{
	//std::cout << "fd tatol count " << channels_.size() << std::endl;

	int numEvents = epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);

	if (numEvents > 0)
	{
		std::cout << numEvents << " events happened" << std::endl;
		fillActiveChannels(numEvents, activeChannels);
		if (static_cast<size_t>(numEvents) == events_.size())
		{
			events_.resize(events_.size() * 2);
		}
	}
	else if (numEvents == 0)
	{
		//std::cout  <<" Thread: "<<currentThread::tid() << " nothing happened" << std::endl;
	}
	else
	{
		if (errno != EINTR)
		{
			//std::cout << " epoll_wait errno happened" << std::endl;
		}
	}
}

void Epoll::fillActiveChannels(int numEvents, ChannelList* activeChannels)const
{
	// assert(static_cast<size_t>(numEvents) <= events_.size());
	for (int i = 0; i < numEvents; ++i)
	{
		Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
		int fd = channel->fd();
		ChannelMap::const_iterator it = channels_.find(fd);
		assert(it != channels_.end());
		assert(it->second == channel);
		channel->set_revents(events_[i].events);
		activeChannels->push_back(channel);
	}
}
void Epoll::updateChannel(Channel* channel)
{
	const int index = channel->index();

	// std::cout << "fd = " << channel->fd() << " events " << channel->events() << " index = " << index ; 
	if (index == kNew || index == kDeleted)
	{
		int fd = channel->fd();
		if (index == kNew)
		{
			assert(channels_.find(fd) == channels_.end());  
			channels_[fd] = channel;
		}
		else
		{
			assert(channels_.find(fd) != channels_.end());
			assert(channels_[fd] == channel);
		}
		//std::cout << "Thread: " << ownerLoop_->threadId_ << " EPOLL_CTL_ADD " << " fd =" << fd << std::endl;
		channel->set_index(kAdded);
		update(EPOLL_CTL_ADD, channel);
		//std::cout << "EPOLL_CTL_ADD" << std::endl;
	}
	else
	{
		// update existing one with EPOLL_CTL_MOD/DEL
		int fd = channel->fd();
		(void)fd;
		assert(channels_.find(fd) != channels_.end());
		assert(channels_[fd] == channel);
		assert(index == kAdded);
		if (channel->isNoneEvent())
		{
			
			//std::cout << "Thread: " << ownerLoop_->threadId_ << " EPOLL_CTL_DEL " << " fd =" << fd << std::endl; 
			update(EPOLL_CTL_DEL, channel);
			channel->set_index(kDeleted);

			//std::cout << "EPOLL_CTL_DEL" << std::endl;
		}
		else
		{
			//std::cout << "Thread: " << ownerLoop_->threadId_ << " EPOLL_CTL_MOD " << " fd =" << fd << std::endl; 
			update(EPOLL_CTL_MOD, channel);
			//std::cout << "EPOLL_CTL_MOD" << std::endl;
		}
	}
}

void Epoll::removeChannel(Channel* channel)
{
	Epoll::assertInLoopThread();
	int fd = channel->fd();
	// std::cout << "Thread: " << ownerLoop_->threadId_ << " removeChannel " << " fd = " << fd << std::endl; 
	assert(channels_.find(fd) != channels_.end());  // ц╩спур╣╫
	assert(channels_[fd] == channel);
	assert(channel->isNoneEvent());
	int index = channel->index();
	assert(index == kAdded || index == kDeleted);
	size_t n = channels_.erase(fd);
	(void)n;
	assert(n == 1);

	if (index == kAdded)
	{
		update(EPOLL_CTL_DEL, channel);
	}
	channel->set_index(kNew);
}

void Epoll::update(int operation, Channel* channel)
{
	struct epoll_event event;
	event.events = channel->events();
	event.data.ptr = channel;
	int fd = channel->fd();
	//std::cout << "epoll_ctl op = " << channel->index()
	//	<< " fd = " << fd << " event = { " << channel->events() << " }" << std::endl;
	if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
	{
		if (operation == EPOLL_CTL_DEL)
		{
			// std::cout << "Thread: " << ownerLoop_->threadId_ << " EPOLL_CTL_ADD " << " fd =" << fd << std::endl;
		}
		else if (operation == EPOLL_CTL_ADD)
		{
			// std::cout << "Thread: " << ownerLoop_->threadId_ << " EPOLL_CTL_ADD " << " fd =" << fd << std::endl;
		}
	}
}

bool Epoll::hasChannel(Channel* channel) const
{
	ChannelMap::const_iterator it = channels_.find(channel->fd());
	return it != channels_.end() && it->second == channel;
}