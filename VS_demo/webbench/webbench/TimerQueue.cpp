// @Author Senwang
// @Email senwang94@gmail.com

#include "TimerQueue.h"
#include <sys/timerfd.h>
#include <iostream>
#include <assert.h>

int createTimerfd()
{
	int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if (timerfd < 0)
	{
		std::cout << "Fail in timerfd_create" << std::endl;
	}
	return timerfd;
}
void resetTimerfd(int timerfd, double mirocseconds = 10000000)
{
	/*
	struct timespec ts;
	ts.tv_sec = static_cast<time_t>(mirocseconds / Timer::kMicroSecondsPerSecond);
	ts.tv_nsec = static_cast<long>((static_cast<int64_t>(mirocseconds) % Timer::kMicroSecondsPerSecond) * 1000);
	struct itimerspec newValue;
	struct itimerspec oldValue;
	bzero(&newValue, sizeof newValue);
	bzero(&oldValue, sizeof oldValue);
	newValue.it_value = ts;
	int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
	*/

	struct itimerspec newValue;
	struct itimerspec oldValue;
	bzero(&newValue, sizeof newValue);
	bzero(&oldValue, sizeof oldValue);
	newValue.it_value.tv_sec = 10;
	newValue.it_value.tv_nsec = 0;
	int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
	// if (ret)
	// {
		// std::cout << "timerfd_settime()" << std::endl;
	// }
}

void readTimerfd(int timerfd)
{
	uint64_t howmany;
	ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
	// std::cout << "TimerQueue::handleRead() " << howmany << " at " << Timer::now() << std::endl;
	if (n != sizeof howmany)
	{
		// std::cout << "TimerQueue::handleRead() reads " << n << " bytes instead of 8" << std::endl;
	}
}
int TimerQueue::count = 0;

TimerQueue::TimerQueue(EventLoop* loop)
	:timerfd_(createTimerfd()),
	loop_(loop),
	timerfdChannel_(new Channel(loop, timerfd_))
{
	// std::cout << "---------start timer-----------" << std::endl;
	timerfdChannel_->setReadCallback(std::bind(&TimerQueue::handleRead, this));
	timerfdChannel_->enableReading();
	resetTimerfd(timerfd_);
}
TimerQueue::~TimerQueue()
{
	timerfdChannel_->disableAll();
	timerfdChannel_->remove();
	::close(timerfd_);
	timers_.clear();
}
void TimerQueue::addTimer(std::shared_ptr<Connection> con, double mirocseconds)
{
	loop_->assertInLoopThread();
	TimerPtr timer(new Timer(con, mirocseconds));
	// loop_->runInLoop(
	// 	std::bind(&TimerQueue::addTimerInLoop, this, timer));
	// addTimerInLoop(timer);

	timers_[timer->get_expiration()] = timer;
}

void TimerQueue::addTimerInLoop(TimerPtr timer)
{
	loop_->assertInLoopThread();
	if (timer->isvalid())
	{
		timers_[timer->get_expiration()] =  timer;
	}
}
void TimerQueue::handleRead()
{
	// std::cout << "-------handle timer----------" << std::endl;
	loop_->assertInLoopThread();
	readTimerfd(timerfd_);
	int64_t now_time = Timer::now();
	// std::cout << "-----" << now_time << "-----" << std::endl;
	for (auto iter = timers_.begin(); iter != timers_.end(); ++iter)
	{
		// std::cout <<" timers: "<< "-----" << (*iter).first << "-----" << std::endl;
		if ((*iter).first <= now_time)
			timers_.erase(iter);
		else break;
	}
	std::cout << count++ << std::endl;
	resetTimerfd(timerfd_);
}