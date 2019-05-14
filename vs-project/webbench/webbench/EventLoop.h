// @Author Senwang
// @Email senwang94@gmail.com

#pragma once

#include "Mutex.h"
#include "CurrentThread.h"
#include "TimerQueue.h"

#include <atomic>
#include <functional>
#include <vector>
#include <memory>


class Channel;
class Epoll;
class Connection;
class TimerQueue;


class EventLoop : noncopy
{
public:
	typedef std::function<void()> Functor;

	EventLoop();
	~EventLoop();  // force out-line dtor, for std::unique_ptr members.


	void loop();

	void quit();


	int64_t iteration() const { return iteration_; }

	void runInLoop(Functor cb);
	void queueInLoop(Functor cb);
	// size_t queueSize() const;

	// internal usage
	void wakeup();
	void updateChannel(Channel* channel);  // 将需要轮询的socket和channel对象注入poller管理器里
	void removeChannel(Channel* channel);
	bool hasChannel(Channel* channel);

	// pid_t threadId() const { return threadId_; }
	void assertInLoopThread()
	{
		if (!isInLoopThread())
		{
			abortNotInLoopThread();
		}
	}
	bool isInLoopThread() const { return threadId_ == currentThread::tid(); }
	// bool callingPendingFunctors() const { return callingPendingFunctors_; }
	bool eventHandling() const { return eventHandling_; }

	void runTimer(std::shared_ptr<Connection> con);

	static EventLoop* getEventLoopOfCurrentThread();
	const pid_t threadId_;
private:

	void abortNotInLoopThread();
	void handleRead();  // waked up
	void doPendingFunctors();

	void printActiveChannels() const; // DEBUG

	typedef std::vector<Channel*> ChannelList;

	bool looping_; /* atomic */
	std::atomic<bool> quit_;
	bool eventHandling_; /* atomic */
	bool callingPendingFunctors_; /* atomic */
	int64_t iteration_;
	// const pid_t threadId_;

	std::unique_ptr<Epoll> poller_;
	int wakeupFd_;   //用于唤醒功能的文件描述符
			// unlike in TimerQueue, which is an internal class,
			// we don't expose Channel to client.
	std::unique_ptr<Channel> wakeupChannel_;
	std::unique_ptr<TimerQueue> timerqueue_;
	// scratch variables
	ChannelList activeChannels_;  //划痕变量
	Channel* currentActiveChannel_;

	mutable MutexLock mutex_;
	std::vector<Functor> pendingFunctors_;  //保存外部注入的回调函数对象
};


