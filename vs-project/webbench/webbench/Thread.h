// @Author Wang sen
// @Email senwang94@gmail.com

#pragma once

#include "CountDownLatch.h"
#include "noncopy.h"
#include <functional>
#include <memory>
#include <pthread.h>
#include <atomic>
class Thread : noncopy
{
public:
	typedef std::function<void()>ThreadFunc;
	explicit Thread(ThreadFunc, const std::string& name = std::string());

	~Thread();

	void start();

	int join();
	bool started()const { return started_; }

private:
	void setDefaultName();
	bool started_;
	bool joined_;
	pthread_t pthreadId_;
	pid_t      tid_;
	ThreadFunc func_;
	std::string name_;
	CountDownLatch latch_;

	//static int numCreated_;
};



