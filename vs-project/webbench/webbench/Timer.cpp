// @Author Senwang
// @Email senwang94@gmail.com

#include "Timer.h"

Timer::Timer(std::shared_ptr<Connection> con, double mirocseconds)
	:isDeleted_(false),
	con_(con)
{
	std::cout << "timer create" << std::endl;
	microAbsSeconds_ = now() + mirocseconds;
}

Timer::~Timer()
{
	std::cout << "timer shutdown" << std::endl;
	assert(con_);
	con_->shutdown();
}

bool Timer::isvalid()
{
	int64_t tmp = now();
	if (tmp >= microAbsSeconds_)
	{
		isDeleted_ = true;
		return false;
	}
	else
	{
		return true;
	}
}

void Timer::update(double microseconds)
{
	microAbsSeconds_ = microseconds + now();
}