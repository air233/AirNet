#include "../md5.h"
#include <gtest/gtest.h>

TEST(testcase0, md5)
{
	std::string md5 = calculateMD5("hello world");

	std::cout << "md5:" << md5 << std::endl;

	EXPECT_EQ(md5, "5eb63bbbe01eeed093cb22bb8f5acdc3");
}