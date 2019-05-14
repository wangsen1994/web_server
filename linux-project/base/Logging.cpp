// @Author Wang sen
// @Email senwang94@gmail.com

#include "Logging.h"
#include "LogStream.h"
#include <sys/time.h>
#include "AsyncLogging.h"
#include <iostream>

static pthread_once_t once_control_ = PTHREAD_ONCE_INIT;
static AsyncLogging *AsyncLogger_;

std::string Logger::logFileName_ = "Senwang_WebServer.log";


Logger::Impl::Impl(std::string& basename_, int line)
	: stream_(),
	line_(line),
	basename_(basename_)
{
	formatTime();
}

void Logger::Impl::formatTime()
{
	struct timeval tv;
	time_t time;
	char str_t[26] = { 0 };
	gettimeofday(&tv, NULL);
	time = tv.tv_sec;
	struct tm* p_time = localtime(&time);
	strftime(str_t, 26, "%Y-%m-%d %H:%M:%S\n", p_time);
	stream_ << str_t;
}

Logger::Logger(std::string fileName, int line)
	: impl_(fileName, line),
	mutex_()
{ 
	if (AsyncLogger_==NULL)
	{
		MutexGuard lock(mutex_);
		if (AsyncLogger_==NULL)
		{
			AsyncLogger_ = new AsyncLogging(Logger::getLogFileName());
			AsyncLogger_->start();
		}
	}
}

Logger::~Logger()
{
	impl_.stream_ << " -- " << impl_.basename_ << ':' << impl_.line_ << '\n';
	const LogStream::Buffer& buf(stream().buffer());
	AsyncLogger_->append(buf.data(), buf.length());
}
