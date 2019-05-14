// @Author Wang sen
// @Email senwang94@gmail.com

#pragma once

#include "noncopy.h"
#include "Mutex.h"
#include "Thread.h"
#include <string>
#include <assert.h>

class EventLoop;

class EventLoopThread :noncopy
{
public:
	EventLoopThread(const std::string& name = std::string());
	~EventLoopThread();

	EventLoop* startLoop();

private:
	void threadFunc();

	EventLoop* loop_;
	bool exiting_;
	Thread thread_;
	MutexLock mutex_;
	Condition cond_;
};

