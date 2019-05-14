// @Author Wang sen
// @Email senwang94@gmail.com

#include "CountDownLatch.h"

CountDownLatch::CountDownLatch(int count)
	:count_(count),
	mutex_(),
	cond_(mutex_)
{
}
void CountDownLatch::wait()
{
	MutexGuard lock(mutex_);
	while (count_>0)
	{
		cond_.wait();
	}
}
void CountDownLatch::countdown()
{
	MutexGuard lock(mutex_);

	--count_;
	if (count_ == 0)
	{
		cond_.broadcast();
	}
}
int CountDownLatch::getcount() const
{
	MutexGuard lock(mutex_);
	return count_;
}