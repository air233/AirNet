#pragma once

#include <stdio.h>
#include<sstream>


class LogStream
{
public:
	LogStream();
	~LogStream();

	//std::ostringstream& operator<<();

private:
	FILE* m_file;

	std::ostringstream m_oss;
};


class LogOutStream
{
public:
	LogOutStream(LogStream& log);
	~LogOutStream();
};
