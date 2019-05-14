// @Author Wang sen
// @Email senwang94@gmail.com

#pragma once
#include "Mutex.h"
#include "noncopy.h"
#include <pthread.h>
#include <time.h>
#include <errno.h>
class Condition
{
public:
	Condition(MutexLock& mutex):mutex_(mutex)
	{
		pthread_cond_init(&cond_, NULL);
	}
	~Condition()
	{
		pthread_cond_destroy(&cond_);
	}
	void wait()
	{
		pthread_cond_wait(&cond_, mutex_.get());
	}
	bool timedwait(int seconds)
	{
		struct timespec time;
		clock_gettime(CLOCK_REALTIME, &time);
		time.tv_sec += seconds;
		return ETIMEDOUT==pthread_cond_timedwait(&cond_, mutex_.get(), &time);
	}
	void signal()
	{
		pthread_cond_signal(&cond_);
	}
	void broadcast()
	{
		pthread_cond_broadcast(&cond_);
	}

private:
	pthread_cond_t cond_;
	MutexLock& mutex_;
};

