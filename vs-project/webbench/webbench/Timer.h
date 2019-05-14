// @Author Senwang
// @Email senwang94@gmail.com

#pragma once
#include "noncopy.h"
#include "Connection.h"
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <memory>
#include <assert.h>

class Connection;

class Timer :public std::enable_shared_from_this<Timer>
{
public:
	Timer(std::shared_ptr<Connection>  con, double mirocseconds);
	~Timer();
	bool isvalid();
	void update(double microseconds);
	int64_t get_expiration()
	{
		return microAbsSeconds_;
	}
	bool isDeleted()
	{
		return isDeleted_;
	}
	static int64_t now()
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		int64_t seconds = tv.tv_sec;
		return seconds * kMicroSecondsPerSecond + tv.tv_usec;
	}
	static const int kMicroSecondsPerSecond = 1000 * 1000;
private:


	int64_t microAbsSeconds_;
	bool isDeleted_;
	std::shared_ptr<Connection> con_;
};
