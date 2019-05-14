// @Author Wang sen
// @Email senwang94@gmail.com

#pragma once
#include "MutexLock.h"
#include "CountDownLatch.h"
#include "Thread.h"
#include "LogStream.h"
#include <string>
#include <vector>
#include <atomic>


class AsyncLogging:noncopy 
{
public:
	AsyncLogging(const std::string& name, int flushInterval = 3);
	~AsyncLogging()
	{
		if (running_)
		{
			stop();
		}
	}
	void append(const char* logline, int len);
	void start()
	{
		running_ = true;
		thread_.start();
		latch_.wait();
	}
	void stop()
	{
		running_ = false;
		cond_.signal();
		thread_.join();
	}

private:
	void threadFunc();

	typedef FixedBuffer<kLargeBuffer> Buffer;
	typedef std::vector<std::unique_ptr<Buffer>>BufferVector;
	typedef BufferVector::value_type BufferPtr;
	//typedef std::vector<std::shared_ptr<Buffer>>BufferVector;
	//typedef std::shared_ptr<Buffer> BufferPtr;

	const int flushInterval_;
	std::atomic<bool> running_;
	const std::string basename_;
	Thread thread_;
	CountDownLatch latch_;
	MutexLock mutex_;
	Condition cond_;
	BufferPtr currentBuffer_;
	BufferPtr nextBuffer_;
	BufferVector buffers_;
};
