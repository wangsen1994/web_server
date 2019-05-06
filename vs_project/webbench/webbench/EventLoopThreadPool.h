// @Author Wang sen
// @Email senwang94@gmail.com


#pragma once
#include "noncopy.h"
#include "EventLoop.h"
#include "EventLoopThread.h"

#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <assert.h>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool :noncopy
{
public:
	EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg);
	~EventLoopThreadPool();
	void setThreadNum(int numThreads) { numThreads_ = numThreads; }
	void start();

	// round-robin
	EventLoop* getNextLoop();

private:

	EventLoop* baseloop_;
	std::string name_;
	bool started_;
	int numThreads_;
	int next_;

	std::vector<std::unique_ptr<EventLoopThread>>threads_;
	std::vector<EventLoop*> loops_;
};

