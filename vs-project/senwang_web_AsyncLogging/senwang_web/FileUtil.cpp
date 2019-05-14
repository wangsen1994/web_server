// @Author Wang sen
// @Email senwang94@gmail.com

#include "FileUtil.h"
#include <string>
#include <assert.h>

AppendFile::AppendFile(const std::string filename)
	:fp_(::fopen(filename.c_str(),"ae")), // 'e' for O_CLOEXEC. close file after exec by other threads
	writtenBytes_(0)
{
	assert(fp_);
	::setbuffer(fp_, buffer_, sizeof buffer_);
}

AppendFile::~AppendFile()
{
	::fclose(fp_);
}
void AppendFile::append(const char* logline, const size_t len)
{
	size_t n = write(logline, len);
	ssize_t nremain = len - n;
	while (nremain>0)
	{
		size_t tmp = write(logline + n, nremain);

		if (tmp == 0)
		{
			int err = ferror(fp_);
			if (err)
			{
				fprintf(stderr, "AppendFile::append() failed\n");
			}
			break;
		}
		n += tmp;
		nremain = len - n;
	}
	writtenBytes_ += len;
}
void AppendFile::flush()
{
	::fflush(fp_);
}
size_t AppendFile::write(const char* logline, size_t len)
{
	return ::fwrite_unlocked(logline, 1, len, fp_);
}