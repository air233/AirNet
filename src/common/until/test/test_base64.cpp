#include "../base64.h"
#include <gtest/gtest.h>

TEST(testcase0, base64)
{
	std::string base64 = base64_encode("hello world");
	std::cout << "base64:" << base64 << std::endl;

	EXPECT_EQ(base64, "aGVsbG8gd29ybGQ=");

	std::string backstr = base64_decode(base64);
	std::cout << "back base64:" << backstr << std::endl;
	EXPECT_EQ(backstr, "hello world");
}