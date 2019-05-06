// @Author Wang sen
// @Email senwang94@gmail.com

#pragma once
#include "noncopy.h"
#include "Mutex.h"
#include "Condition.h"

class CountDownLatch
{
public:
	explicit CountDownLatch(int count);
	void wait();
	void countdown();
	int getcount() const;

private:
	int count_;
	mutable MutexLock mutex_;
	Condition cond_;
};


