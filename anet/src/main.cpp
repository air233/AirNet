#include "common/buffer/buffer.h"
#include <iostream>

int main()
{
	Buffer buff;

	buff.pushInt32(123);

	int32_t out;
	buff.peekInt32(out);

	std::cout << "out:" << out << std::endl;
}

