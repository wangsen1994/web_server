// @Author Wang sen
// @Email senwang94@gmail.com

#include "Thread.h"
#include "CurrentThread.h"

#include <sys/prctl.h>
#include <sys/syscall.h>
#include <linux/unistd.h>
#include <unistd.h>
#include <assert.h>

namespace currentThread
{
	__thread int t_cachedTid = 0;  // 每一个线程有一份独立实体 只能修饰POD类型
	__thread char t_tidString[32];
	__thread int t_tidStringLength = 6;
	__thread const char* t_threadName = "unknown";
}

pid_t gettid()
{
	return static_cast<pid_t>(::syscall(SYS_gettid));
}
void currentThread::cacheTid()
{
	if (t_cachedTid == 0)
	{
		t_cachedTid = gettid();
		t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
	}
}

struct ThreadData
{
	typedef Thread::ThreadFunc ThreadFunc;
	ThreadFunc func_;
	std::string name_;
	pid_t* tid_;
	CountDownLatch* latch_;

	ThreadData(ThreadFunc func, const std::string& name, pid_t* tid, CountDownLatch* latch)
		:func_(func),
		name_(name),
		tid_(tid),
		latch_(latch)
	{}
	void runInThread()
	{
		*tid_ = currentThread::tid();
		tid_ = NULL;
		latch_->countdown();
		latch_ = NULL;
		currentThread::t_threadName = name_.empty() ? "Thread" : name_.c_str();
		prctl(PR_SET_NAME, currentThread::t_threadName); // set thread name

		func_();
		currentThread::t_threadName = "finished";

	}
};
Thread::Thread(ThreadFunc func, const std::string& n)
	:started_(false),
	joined_(false),
	pthreadId_(0),
	tid_(0),
	func_(func),
	name_(n),
	latch_(1)
{
	setDefaultName();
}
Thread::~Thread()
{
	if (started_ && !joined_)
	{
		pthread_detach(pthreadId_);
	}
}
void Thread::setDefaultName()
{
	//numCreated_++;
	if (name_.empty())
	{
		char buf[32];
		//snprintf(buf, sizeof buf, "Thread%d", numCreated_);
		snprintf(buf, sizeof buf, "Thread");
		name_ = buf;
	}
}
void* startThread(void* obj)
{
	ThreadData* data = static_cast<ThreadData*>(obj);
	data->runInThread();
	delete data;
	return NULL;
}
void Thread::start()
{
	assert(!started_);
	started_ = true;
	ThreadData* data = new ThreadData(func_, name_, &tid_, &latch_);
	if (pthread_create(&pthreadId_, NULL, &startThread, data))
	{
		started_ = false;
		delete data;
		fprintf(stderr, "pthread error");
	}
	else
	{
		latch_.wait();
		assert(tid_ > 0);
	}
}
int Thread::join()
{
	assert(started_);
	assert(!joined_);
	joined_ = true;
	return pthread_join(pthreadId_, NULL);
}