#include "log.h"
#include <stdio.h>
#include <iostream>
#include "../until/times.h"

Log::Log(std::string& log_path) :
	DEBUG(m_stream),INFO(m_stream),WRAN(m_stream),ERROR(m_stream),FATAL(m_stream)
{
	::fopen_s(&m_file, log_path.c_str(), "a");
}

Log::~Log()
{
}

void Log::flush()
{
	std::cout << m_oss.str().c_str() << std::endl;

	fprintf(m_file, m_oss.str().c_str());
	fprintf(m_file, "\n");

	m_oss.str("");
	m_oss.clear();
}

std::ostringstream& Log::strem()
{
	m_oss <<"[" << GetMSTimeStr() << "]";

	return m_oss;
}

LogStream& Log::logstream()
{
	return m_stream;
}

void Log::debug(std::string log)
{

}

