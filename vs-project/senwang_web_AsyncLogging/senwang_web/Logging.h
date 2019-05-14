// @Author Wang sen
// @Email senwang94@gmail.com

#pragma once
#include "LogStream.h"
#include "AsyncLogging.h"
#include <string>

class AsyncLogging;

class Logger
{
public:
	Logger(std::string logFile, int line);
	~Logger();
	LogStream& stream() { return impl_.stream_; }

	static void setLogFileName(std::string fileName)
	{
		logFileName_ = fileName;
	}
	static std::string getLogFileName()
	{
		return logFileName_;
	}

private:
	class Impl
	{
	public:
		Impl(std::string& basename_, int line);

		void formatTime();
		LogStream stream_;
		int line_;
		const std::string basename_;
	};

	Impl impl_;
	static std::string logFileName_;
	static AsyncLogging* AsyncLogging_;
	MutexLock mutex_;
};
#define LOG Logger(__FILE__, __LINE__).stream()

