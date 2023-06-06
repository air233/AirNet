#include "log.h"
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include "../until/times.h"

Log::Log(const std::string& log_path, const std::string& file_name, int32_t level, int32_t type):
	m_file_stream(log_path, file_name, 0, 0, 0),m_level(level),m_type(type)
{
}

Log::~Log()
{
}


FileStream& Log::logstream()
{
	return m_file_stream;
}


void Log::debug(const char* format, ...)
{
	if (m_level > 0) return;

	va_list args;
	va_start(args, format);
	_log("DEBUG", format, args);
	va_end(args);
}

void Log::info(const char* format, ...)
{
	if (m_level > 1) return;

	va_list args;
	va_start(args, format);
	_log("INFO", format, args);
	va_end(args);
}

void Log::warn(const char* format, ...)
{
	if (m_level > 2) return;

	va_list args;
	va_start(args, format);
	_log("WARN", format, args);
	va_end(args);
}

void Log::error(const char* format, ...)
{
	if (m_level > 3) return;

	va_list args;
	va_start(args, format);
	_log("ERROR", format, args);
	va_end(args);
}

void Log::fatal(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	_log("FATAL", format, args);
	va_end(args);

	abort();
}

void Log::log(const char* level, const char* pszFileName, const char* pszFuncName, int32_t nLineNum, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	_log(level,pszFileName,pszFuncName,nLineNum, format, args);
	va_end(args);

	if (level == "FATAL")
	{
		abort();
	}
}

void Log::_log(const char* level, const char* format, va_list argptr)
{
	char szMsg[1024] = { 0 };
	vsnprintf(szMsg, sizeof(szMsg), format, argptr);
	m_file_stream.log(level, szMsg);
}

void Log::_log(const char* level, const char* pszFileName, const char* pszFuncName, int32_t nLineNum, const char* format, va_list argptr)
{
	char szMsg[1024] = { 0 };

	int32_t n1 = 0;

	if (m_type == 1)
	{
		n1 = snprintf(szMsg, sizeof szMsg, "(%s:%d:%s) ", pszFileName, nLineNum, pszFuncName);
	}

	//TODO:后续迭代 此处打印格式可定制
	
	vsnprintf(szMsg+n1, sizeof(szMsg)-n1, format, argptr);
	m_file_stream.log(level, szMsg);
}
