// @Author Wang sen
// @Email senwang94@gmail.com

#include "EventLoopThreadPool.h"
#include <iostream>

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg)
	:baseloop_(baseLoop),
	name_(nameArg),
	started_(false),
	next_(0),
	numThreads_(0)
{}
EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::start()
{
	assert(!started_);
	baseloop_->assertInLoopThread();
	started_ = true;

	for (int i = 0; i < numThreads_; ++i)
	{
		char buf[name_.size() + 32];
		snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
		EventLoopThread* t = new EventLoopThread(buf);
		threads_.push_back(std::unique_ptr<EventLoopThread>(t));
		loops_.push_back(t->startLoop());
	}
	// if (numThreads_ == 0)
		// std::cout << "Threads number is 0" << std::endl;
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
	baseloop_->assertInLoopThread();         // 若loops_为0，则返回主线程loop
	assert(started_);
	EventLoop* loop = baseloop_;

	if (!loops_.empty())
	{
		// round-robin
		loop = loops_[next_];
		++next_;
		if (static_cast<size_t>(next_) >= loops_.size())
		{
			next_ = 0;
		}
	}
	return loop;
}