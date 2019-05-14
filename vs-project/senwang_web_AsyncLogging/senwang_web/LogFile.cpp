// @Author Wang sen
// @Email senwang94@gmail.com

#include "LogFile.h"
#include <string>
#include <assert.h>

LogFile::LogFile(const std::string& basename, int flushInterval)
	:basename_(basename),
	mutex_(new MutexLock),
	count_(0),
	flushInterval_(flushInterval)
{
	file_.reset(new AppendFile(basename_));
}
LogFile::~LogFile() = default;

void LogFile::append(const char* logline, int len)
{
	MutexGuard lock(*mutex_);
	append_unlocked(logline, len);
}
void LogFile::flush()
{
	MutexGuard lock(*mutex_);
	file_->flush();
}
void LogFile::append_unlocked(const char* logline, int len)
{
	file_->append(logline, len);
	count_++;
	if (count_ > flushInterval_)
	{
		count_ = 0;
		file_->flush();
	}
}