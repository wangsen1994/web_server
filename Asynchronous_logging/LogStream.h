// @Author Wang sen
// @Email senwang94@gmail.com

#pragma once
#include "noncopy.h"
#include <assert.h>
#include <string.h>
#include <string>

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template<int SIZE>
class FixedBuffer:noncopy
{
public:
	FixedBuffer()
		:cur_(data_)
	{}
	~FixedBuffer() {}

	void append(const char* buf, size_t len)
	{
		if (static_cast<size_t>(avail()) > len)
		{
			memcpy(cur_, buf, len);
			cur_ += len;
		}
	}
	const char* data()const { return data_; }

	int length() const { return static_cast<int>(cur_ - data_); }
	char* current() { return cur_; }
	int avail()const { return static_cast<int>(end() - cur_); }

	void add(size_t len) { cur_ += len; }
	void reset() { cur_ = data_; }
	void bzero() { memset(data_, 0, sizeof data_); }

private:
	const char* end() const { return data_ + sizeof data_; }
	
	char data_[SIZE];
	char* cur_;
};

class LogStream:noncopy 
{
public:
	typedef LogStream Stream;
	typedef FixedBuffer<kSmallBuffer> Buffer;
public:

	Stream& operator<<(bool v)
	{
		buffer_.append(v ? "1" : "0", 1);
		return *this;
	}
	Stream& operator<<(short);
	Stream& operator<<(unsigned short);
	Stream& operator<<(int);
	Stream& operator<<(unsigned int);
	Stream& operator<<(long);
	Stream& operator<<(unsigned long);
	Stream& operator<<(long long);
	Stream& operator<<(unsigned long long);

	Stream& operator<<(const void*);

	Stream& operator<<(float v)
	{
		*this << static_cast<double>(v);
		return *this;
	}
	Stream& operator<<(double);
	Stream& operator<<(long double);
	Stream& operator<<(char v)
	{
		buffer_.append(&v, 1);
		return *this;
	}

	Stream& operator<<(const char* str)
	{
		if (str)
		{
			buffer_.append(str, strlen(str));
		}
		else
		{
			buffer_.append("(null)", 6);
		}
		return *this;
	}

	Stream& operator<<(const unsigned char* str)
	{
		return operator<<(reinterpret_cast<const char*>(str));
	}

	Stream& operator<<(const std::string& v)
	{
		buffer_.append(v.c_str(), v.size());
		return *this;
	}

	void append(const char* data, int len) { buffer_.append(data, len); }
	const Buffer& buffer() const { return buffer_; }
	void resetBuffer() { buffer_.reset(); }

private:
	template<typename T>
	void formatInteger(T);

	Buffer buffer_;
	static const int kMaxNumericSize = 32;
};



