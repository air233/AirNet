#include <iostream>
#include "../log.h"
#include "../filestream.h"
#include "../../until/times.h"

int main()
{
	Log log;
	log.debug("%s", "hello world");
	log.log("DEBUG", __FILE__, __FUNCTION__, __LINE__, "this new test : %u", 12332132);

	DEBUG("now time is %s", GetTimeStr().c_str());

	LOG_DEBUG << "this stream log .time:" << GetTimeStr();
	LOG_DEBUG << "stream new line. num:" << 123;

	return 0;
}