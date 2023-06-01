#include "../times.h"
#include <gtest/gtest.h>
#include <string>

TEST(testcase0, time)
{
	uint32_t time = GetTime();
	std::cout << "get seconds time:" << time << std::endl;

	uint64_t ms_time = GetMSTime();
	std::cout << "get milliseconds time:" << ms_time << std::endl;

	uint32_t day = GetWeekDay();
	std::cout << "get week day1:" << day << std::endl;

	uint32_t day2 = GetWeekDay(time);
	std::cout << "get week day2:" << day2 << std::endl;

	std::string cur = GetTimeStr();
	std::cout << "cur seconds:" << cur << std::endl;

	std::string curms = GetMSTimeStr();
	std::cout << "cur milliseconds:" << curms << std::endl;

	

	EXPECT_EQ(day, day2);
}