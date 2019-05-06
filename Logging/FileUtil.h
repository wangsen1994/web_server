// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#pragma once
#include "noncopyable.h"
#include <string>

class AppendFile : noncopyable
{
public:
	explicit AppendFile(std::string filename);
	//explicit AppendFile(char* filename);
	~AppendFile();
	// append �����ļ�д
	void append(const char *logline, const size_t len);
	void flush();

private:
	size_t write(const char *logline, size_t len);
	FILE* fp_;
	char buffer_[32 * 1024];
};