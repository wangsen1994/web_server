// @Author Wang sen
// @Email senwang94@gmail.com

#include "Logging.h"
#include <string>
#include "Thread.h"
#include <cstdio>
#include <unistd.h>
#include <iostream>

using namespace std;

void threadFunc()
{
	for (int i = 0; i < 100000; ++i)
	{
		LOG << i;
	}
}
void type_test()
{
	// 13 lines
	cout << "----------type test-----------" << endl;
	LOG << 0;
	LOG << 1234567890123;
	LOG << 1.0f;
	LOG << 3.1415926;
	LOG << (short)1;
	LOG << (long long)1;
	LOG << (unsigned int)1;
	LOG << (unsigned long)1;
	LOG << (long double) 1.6555556;
	LOG << (unsigned long long) 1;
	LOG << 'c';
	LOG << "abcdefg";
	LOG << string("This is a string");
}
void stressing_single_thread()
{
	// 100000 lines
	cout << "----------stressing test single thread-----------" << endl;
	for (int i = 0; i < 100000; ++i)
	{
		LOG << i;
	}
}

void stressing_multi_threads(int threadNum = 4)
{
	// threadNum * 100000 lines
	cout << "----------stressing test multi thread-----------" << endl;
	vector<shared_ptr<Thread>> vsp;
	for (int i = 0; i < threadNum; ++i)
	{
		shared_ptr<Thread> tmp(new Thread(threadFunc, "testFunc"));
		vsp.push_back(tmp);
	}
	for (int i = 0; i < threadNum; ++i)
	{
		vsp[i]->start();
	}
	sleep(3);
}
int main()
{
	type_test();
	sleep(3);

	stressing_single_thread();
	sleep(3);

	stressing_multi_threads();
	sleep(3);
}