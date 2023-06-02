#include <iostream>
#include "../log.h"

int main()
{
	Log log;

	log.strem() << "hello world";
	log.flush();


	log.strem() << "test test";
	log.flush();

	log.debug("hello wold");

	return 0;
}