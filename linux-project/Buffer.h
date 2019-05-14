// @Author Senwang
// @Email senwang94@gmail.com

#pragma once
#include <algorithm>
#include <vector>
#include <assert.h>
#include <string.h>


/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode


class Buffer
{
public:

	static const size_t kCheapPrepend = 8;
	static const size_t kInitialSize = 1024;

	explicit Buffer(size_t iniSize = kInitialSize)
		:buffer_(kCheapPrepend + iniSize),
		readerIndex_(kCheapPrepend),
		writerIndex_(kCheapPrepend)
	{
	}

	void swap(Buffer& tmp)
	{
		buffer_.swap(tmp.buffer_);
		std::swap(readerIndex_, tmp.readerIndex_);
		std::swap(writerIndex_, tmp.writerIndex_);
	}

	size_t readableBytes() const   //�ɶ���������
	{
		return writerIndex_ - readerIndex_;
	}
	size_t writableBytes() const   //��д�Ŀռ�
	{
		return buffer_.size() - writerIndex_;
	}
	size_t prependableBytes() const  //ǰ����пռ�
	{
		return readerIndex_;
	}

	void prepend(const void* data, size_t len)   //��ͷ�����
	{
		assert(len <= prependableBytes());
		readerIndex_ -= len;
		const char* d = static_cast<const char*>(data);
		std::copy(d, d + len, begin() + readerIndex_);
	}
	const char* peek() const         // �ɶ���λ�÷��±�
	{
		return begin() + readerIndex_;
	}
	char* get_pos()
	{
		return pos;
	}
	void set_pos(char* position)
	{
		pos = position;
	}

	const char* findCRLF() const     //���з�
	{
		// FIXME: replace with memmem()?
		const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
		return crlf == beginWrite() ? NULL : crlf;
	}
	const char* findCRLF(const char* start) const
	{
		assert(peek() <= start);
		assert(start <= beginWrite());
		// FIXME: replace with memmem()?
		const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
		return crlf == beginWrite() ? NULL : crlf;
	}

	const char* findEOL() const          //�ļ���β��
	{
		const void* eol = memchr(peek(), '\n', readableBytes());
		return static_cast<const char*>(eol);
	}

	const char* findEOL(const char* start) const
	{
		assert(peek() <= start);
		assert(start <= beginWrite());
		const void* eol = memchr(start, '\n', beginWrite() - start);
		return static_cast<const char*>(eol);
	}

	void append(const char* data, size_t len)
	{
		ensureWritableBytes(len);
		std::copy(data, data + len, beginWrite());
		hasWritten(len);
	}
	void ensureWritableBytes(size_t len)
	{
		if (writableBytes() < len)
		{
			makeSpace(len);
		}
		assert(writableBytes() >= len);
	}
	void append(const void* data, size_t len)
	{
		append(static_cast<const char*>(data), len);
	}

	char* beginWrite()                //��ʼ��д��λ��
	{
		return begin() + writerIndex_;
	}

	const char* beginWrite() const    //��ʼ��д��λ��
	{
		return begin() + writerIndex_;
	}

	void hasWritten(size_t len)
	{
		assert(len <= writableBytes());
		writerIndex_ += len;
	}

	void unwrite(size_t len)       // ��дlen�����ֶ�
	{
		assert(len <= readableBytes());
		writerIndex_ -= len;
	}
	void retrieve(size_t len)
	{
		assert(len <= readableBytes());   // �ͷſɶ�����
		if (len < readableBytes())
		{
			readerIndex_ += len;
		}
		else
		{
			retrieveAll();
		}
	}
	void retrieveAll()   //��ԭ
	{
		readerIndex_ = kCheapPrepend;
		writerIndex_ = kCheapPrepend;
		buffer_.resize(kInitialSize + kCheapPrepend);
	}
	void retrieveUntil(const char* end)
	{
		assert(peek() <= end);
		assert(end <= beginWrite());
		retrieve(end - peek());
	}

	// ֱ�Ӷ���buffer
	ssize_t readFd(int fd, int* savedErrno);


private:

	char* begin()
	{
		return &*buffer_.begin();
	}
	const char* begin() const
	{
		return &*buffer_.begin();
	}

	void makeSpace(size_t len)
	{
		if (writableBytes() + prependableBytes() < len + kCheapPrepend)  //������8���ֽڵ�ͷ��
		{
			// FIXME: move readable data
			buffer_.resize(writerIndex_ + len);
		}
		else
		{
			// move readable data to the front, make space inside buffer
			assert(kCheapPrepend < readerIndex_);
			size_t readable = readableBytes();
			std::copy(begin() + readerIndex_,
				begin() + writerIndex_,
				begin() + kCheapPrepend);
			readerIndex_ = kCheapPrepend;
			writerIndex_ = readerIndex_ + readable;
			assert(readable == readableBytes());
		}
	}

	std::vector<char> buffer_;
	size_t readerIndex_;
	size_t writerIndex_;

	char* pos;
	static const char kCRLF[];
};

