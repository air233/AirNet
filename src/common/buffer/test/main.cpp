#include <iostream>
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
	std::cout << "run buffer test --> " << std::endl << std::endl;

	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
