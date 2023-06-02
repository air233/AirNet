#include "times.h"
#include <chrono>
#include <time.h>
#include <iostream>
#include <sstream>
#include <iomanip>


static const int kMicroSecondsPerSecond = 1000 * 1000;
static const int kMilliSecondsPerSecond = 1000;


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

	std::tm tm;
	localtime_s(&tm, &t);

	char buffer[80];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);

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

	std::tm tm;
	localtime_s(&tm, &st);
	char buffer[80];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);

	std::ostringstream oss;
	oss << buffer << "." << ms;
	return oss.str();
}

uint32_t GetWeekDay()
{
	auto now = std::chrono::system_clock::now();
	std::time_t time = std::chrono::system_clock::to_time_t(now);
	std::tm tm;
	localtime_s(&tm,&time);

	// tm_wday 表示星期几，其中 0 表示星期日，1 表示星期一，以此类推
	return tm.tm_wday;
}

uint32_t GetWeekDay(uint32_t time)
{
	std::time_t t = time;
	std::tm tm;
	localtime_s(&tm, &t);

	// tm_wday 表示星期几，其中 0 表示星期日，1 表示星期一，以此类推
	return tm.tm_wday;
}

