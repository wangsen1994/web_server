// @Author Senwang
// @Email senwang94@gmail.com

#pragma once
#include "base/noncopy.h"
#include "Timer.h"
#include "Connection.h"
#include "EventLoop.h"
#include "Channel.h"
#include <set>
#include <memory>
#include <unistd.h>
#include <map>

class Channel;
class Connection;
class Timer;
class EventLoop;

class TimerQueue :noncopy
{
public:
	TimerQueue(EventLoop* loop);
	~TimerQueue();


	void addTimer(std::shared_ptr<Connection> con, double mirocseconds = 0);

	void handleRead();

private:

	typedef std::shared_ptr<Timer> TimerPtr;
	typedef std::map<int64_t, TimerPtr> TimerList;

	void addTimerInLoop(TimerPtr timer);


	static int count;
	TimerList timers_;
	const int timerfd_;
	EventLoop* loop_;
	std::unique_ptr<Channel> timerfdChannel_;
};


