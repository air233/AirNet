#include "times.h"
#include <chrono>
#include <time.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif // _WIN32
static const int kMicroSecondsPerSecond = 1000 * 1000;
static const int kMilliSecondsPerSecond = 1000;

time_t GetNow()
{
	auto now = std::chrono::system_clock::now();
	auto milliseconds = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	return milliseconds.time_since_epoch().count();
}

uint32_t GetTime()
{
	auto now = std::chrono::system_clock::now();
	auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	return (uint32_t)seconds.time_since_epoch().count();
}

uint64_t GetMSTime()
{
	auto now = std::chrono::system_clock::now();
	auto milliseconds = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	return milliseconds.time_since_epoch().count();
}

std::string GetTimeStr(uint32_t time)
{
	std::time_t t = time;
	if (t == 0)
	{
		t = GetTime();
	}

	std::tm* tm;
	tm = localtime(&t);

	char buffer[80];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);

	return buffer;
}

std::string GetMSTimeStr(uint64_t mstime)
{
	std::time_t mst = mstime;

	if (mst == 0)
	{
		mst = GetMSTime();
	}

	std::time_t st = mst / kMilliSecondsPerSecond;
	uint64_t ms = mst % kMilliSecondsPerSecond;

	std::tm* tm;
	tm = localtime(&st);
	char buffer[80];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);

	std::ostringstream oss;
	oss << buffer << "." << std::setfill('0') << std::setw(3) << ms;
	return oss.str();
}

uint32_t GetWeekDay()
{
	auto now = std::chrono::system_clock::now();
	std::time_t time = std::chrono::system_clock::to_time_t(now);
	std::tm* tm;
	tm = localtime(&time);

	// tm_wday 表示星期几，其中 0 表示星期日，1 表示星期一，以此类推
	return tm->tm_wday;
}

uint32_t GetWeekDay(uint32_t time)
{
	std::time_t t = time;
	std::tm* tm;
	tm = localtime(&t);

	// tm_wday 表示星期几，其中 0 表示星期日，1 表示星期一，以此类推
	return tm->tm_wday;
}

bool IsSameDay(uint32_t time1, uint32_t time2)
{
	std::tm* tm1, *tm2;

	std::time_t t1 = time1;
	std::time_t t2 = time2;

	tm1 = localtime(&t1);
	tm2 = localtime(&t2);

	return (tm1->tm_year == tm2->tm_year) && 
		(tm1->tm_mon == tm2->tm_mon) && 
		(tm1->tm_mday == tm2->tm_mday);
}

bool IsSameHour(uint32_t time1, uint32_t time2)
{
	std::tm *tm1, *tm2;

	std::time_t t1 = time1;
	std::time_t t2 = time2;

	tm1 = localtime(&t1);
	tm2 = localtime(&t2);

	return (tm1->tm_year == tm2->tm_year) && 
		(tm1->tm_mon == tm2->tm_mon) && 
		(tm1->tm_mday == tm2->tm_mday) && 
		(tm1->tm_hour == tm2->tm_hour);
}

void Wait(uint32_t milliseconds)
{
#ifdef _WIN32
	Sleep(milliseconds);
#else
	struct timespec req;
	req.tv_sec = milliseconds / 1000;
	req.tv_nsec = (milliseconds % 1000) * 1000000;
	nanosleep(&req, NULL);
#endif
}

