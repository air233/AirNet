#include "common/buffer/buffer.h"
#include <iostream>
#include "common/log/log.h"
int main()
{
	Buffer buff;

	buff.pushInt32(123);

	int32_t out;
	buff.peekInt32(out);

	DEBUG("test log system");

	std::cout << "out:" << out << std::endl;
}

