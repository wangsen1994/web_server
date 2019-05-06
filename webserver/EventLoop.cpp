// @Author Senwang
// @Email senwang94@gmail.com

#include "EventLoop.h"
#include "Mutex.h"
#include "Channel.h"
#include "Epoll.h"
#include "Socket.h"

#include <algorithm>
#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <assert.h>
#include <iostream>


__thread EventLoop* t_loopInThisThread = 0;

const int kPollTimeMs = 10000;

int createEventfd()
{
	int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (evtfd < 0)
	{
		//std::cout << "Failed in eventfd" << std::endl;
		abort();
	}
	return evtfd;
}

EventLoop::EventLoop()
	:looping_(false),
	quit_(false),
	eventHandling_(false),
	callingPendingFunctors_(false),
	iteration_(0),
	threadId_(currentThread::tid()),
	poller_(new Epoll(this)),
	wakeupFd_(createEventfd()),
	wakeupChannel_(new Channel(this, wakeupFd_)),
	// timerqueue_(new TimerQueue(this)),
	currentActiveChannel_(NULL)
{
	if (t_loopInThisThread)
	{
		//std::cout << "Another EventLoop " << t_loopInThisThread
		//	<< " exists in this thread " << threadId_ << std::endl;
	}
	else
	{
		t_loopInThisThread = this;
	}
	wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
	wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
	std::cout << "EventLoop " << this << " of thread " << threadId_
		<< " destructs in thread " << currentThread::tid() << std::endl;
	wakeupChannel_->disableAll();
	wakeupChannel_->remove();
	::close(wakeupFd_);
	t_loopInThisThread = NULL;
}

void EventLoop::loop()
{
	assert(!looping_);
	assertInLoopThread();

	looping_ = true;
	quit_ = false;

	//std::cout << "EventLoop " << this << " start looping" << std::endl;

	while (!quit_)
	{
		activeChannels_.clear();
		poller_->poll(kPollTimeMs, &activeChannels_);
		++iteration_;
		eventHandling_ = true;
		for (Channel* channel : activeChannels_)
		{
			currentActiveChannel_ = channel;
			currentActiveChannel_->handleEvent();
		}
		currentActiveChannel_ = NULL;
		eventHandling_ = false;
		doPendingFunctors();
	}

	//std::cout << "EventLoop " << this << " stop looping" << std::endl;
	looping_ = false;
}

void EventLoop::quit()
{
	quit_ = true;
	if (!isInLoopThread())
	{
		wakeup();
	}
}

void EventLoop::runInLoop(Functor cb)
{
	if (isInLoopThread())
	{
		cb();
	}
	else
	{
		queueInLoop(std::move(cb));
	}
}

void EventLoop::queueInLoop(Functor cb)
{
	{
		MutexGuard lock(mutex_);
		pendingFunctors_.push_back(std::move(cb));
	}
	if (!isInLoopThread() || callingPendingFunctors_)
	{
		wakeup();
	}
}

void EventLoop::updateChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	if (eventHandling_)
	{
		assert(currentActiveChannel_ == channel ||
			std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
	}
	poller_->removeChannel(channel);
}
bool EventLoop::hasChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread()
{
	//std::cout << "EventLoop::abortNotInLoopThread - EventLoop " << this
	//	<< " was created in threadId_ = " << threadId_
	//	<< ", current thread id = " << currentThread::tid() << std::endl;
}

void EventLoop::wakeup()
{
	uint64_t one = 1;
	ssize_t n = ::write(wakeupFd_, &one, sizeof one); // ÓÃÀ´»½ÐÑ
	if (n != sizeof one)
	{
		//std::cout << "EventLoop::wakeup() writes " << n << " bytes instead of 8" << std::endl;
	}
}

void EventLoop::handleRead()
{
	uint64_t one = 1;
	ssize_t n = ::read(wakeupFd_, &one, sizeof one);
	if (n != sizeof one)
	{
		//std::cout << "EventLoop::handleRead() reads " << n << " bytes instead of 8" << std::endl;
	}
}

void EventLoop::doPendingFunctors()
{
	std::vector<Functor> functors;
	callingPendingFunctors_ = true;

	{
		MutexGuard lock(mutex_);
		functors.swap(pendingFunctors_);
	}

	for (const Functor& functor : functors)
	{
		functor();
	}
	callingPendingFunctors_ = false;
}
void EventLoop::runTimer(std::shared_ptr<Connection> con)
{
	// timerqueue_->addTimer(con);

}