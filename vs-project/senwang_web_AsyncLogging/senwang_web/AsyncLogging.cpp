// @Author Wang sen
// @Email senwang94@gmail.com

#include "AsyncLogging.h"
#include "LogFile.h"
#include <stdio.h>
#include <assert.h>


AsyncLogging::AsyncLogging(const std::string& name, int flushInterval)
	:flushInterval_(flushInterval),
	running_(false),
	basename_(name),
	thread_(std::bind(&AsyncLogging::threadFunc,this),"Logging"),
	latch_(1),
	mutex_(),
	cond_(mutex_),
	currentBuffer_(new Buffer),
	nextBuffer_(new Buffer),
	buffers_()
{
	currentBuffer_->bzero();
	nextBuffer_->bzero();
	buffers_.reserve(16);
}
void AsyncLogging::append(const char* logline, int len)
{
	
	MutexGuard lock(mutex_);
	if (currentBuffer_->avail() > len)
	{
		currentBuffer_->append(logline, len);
		// cond_.signal();
	}
	else
	{
		buffers_.push_back(std::move(currentBuffer_));
		if (nextBuffer_)
		{
			currentBuffer_ = std::move(nextBuffer_);
		}
		else
		{
			currentBuffer_.reset(new Buffer);
		}
		currentBuffer_->append(logline, len);
		cond_.signal();
	}
}

void AsyncLogging::threadFunc()
{
	assert(running_ == true);
	latch_.countdown();
	LogFile logfile(basename_);
	BufferPtr newBuffer1(new Buffer);
	BufferPtr newBuffer2(new Buffer);
	newBuffer1->bzero();
	newBuffer2->bzero();
	BufferVector bufferTowrite;
	bufferTowrite.reserve(16);

	while (running_)
	{
		assert(newBuffer1 && newBuffer1->length() == 0);
		assert(newBuffer2 && newBuffer2->length() == 0);
		assert(bufferTowrite.empty());

		{
			MutexGuard lock(mutex_); //mutex_来自父线程
			if (buffers_.empty())
			{
				cond_.timedwait(flushInterval_);
			}

			buffers_.push_back(std::move(currentBuffer_));
			currentBuffer_ = std::move(newBuffer1);
			bufferTowrite.swap(buffers_);
			if (!nextBuffer_)
			{
				nextBuffer_ = std::move(newBuffer2);
			}
		}

		assert(!bufferTowrite.empty());
		if (bufferTowrite.size() > 25)
		{
			char buf[256];
			snprintf(buf, sizeof buf, "Dropped log messages, %zd larger buffers\n", bufferTowrite.size() - 2);
			fputs(buf, stderr);
			logfile.append(buf, strlen(buf));
			bufferTowrite.erase(bufferTowrite.begin() + 2, bufferTowrite.end());
		}
		for (const auto& buffer : bufferTowrite)
		{
			logfile.append(buffer->data(), buffer->length());
		}
		if (bufferTowrite.size() > 2)
		{
			// drop non-bzero-ed buffers, avoid trashing
			bufferTowrite.resize(2);
		}

		if (!newBuffer1)
		{
			assert(!bufferTowrite.empty());
			newBuffer1 = std::move(bufferTowrite.back());
			bufferTowrite.pop_back();
			newBuffer1->reset();
		}

		if (!newBuffer2)
		{
			assert(!bufferTowrite.empty());
			newBuffer2 = std::move(bufferTowrite.back());
			bufferTowrite.pop_back();
			newBuffer2->reset();
		}
		bufferTowrite.clear();
		logfile.flush();
	}
	logfile.flush();
}