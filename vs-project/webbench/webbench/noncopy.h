// @Author Wang sen
// @Email senwang94@gmail.com
#pragma once

class noncopy
{

protected:
	noncopy() {}
	~noncopy() {}

private:
	noncopy(const noncopy&) {}
	const noncopy& operator=(const noncopy&) {}
};