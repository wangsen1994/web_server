// @Author Wang sen
// @Email senwang94@gmail.com

#pragma once
#include "noncopy.h"
#include <pthread.h>
#include <errno.h>


class MutexLock:noncopy
{
public:
	MutexLock()
	{
		pthread_mutex_init(&mutex, NULL);

	}
	~MutexLock()
	{
		pthread_mutex_destroy(&mutex);
	}
	void lock()
	{
		pthread_mutex_lock(&mutex);
	}
	void unlock()
	{
		pthread_mutex_unlock(&mutex);
	}
	pthread_mutex_t* get()
	{
		return &mutex;
	}
private:
	pthread_mutex_t mutex;
private:
	friend class Condition;
};

class MutexGuard:noncopy
{
public:
	MutexGuard(MutexLock& mutex):mutex_(mutex)
	{
		mutex_.lock();
	}
	~MutexGuard()
	{
		mutex_.unlock();
	}

private:
	MutexLock& mutex_;
};


