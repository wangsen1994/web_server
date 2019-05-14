// @Author Wang sen
// @Email senwang94@gmail.com

#pragma once
#include "Mutex.h"
#include "noncopy.h"
#include <string>
#include <memory>
#include "FileUtil.h"
class LogFile:noncopy
{
public:
	LogFile(const std::string& basename, int flushInterval = 3);
	~LogFile();
	void append(const char* logline, int len);
	void flush();
private:
	void append_unlocked(const char* logline, int len);


	const std::string basename_;
	const int flushInterval_;
	int count_;

	std::unique_ptr<MutexLock> mutex_;
	std::unique_ptr<AppendFile> file_;

};
