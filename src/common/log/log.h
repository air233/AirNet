#pragma once
#include<string>
#include <stdio.h>
#include<sstream>

#include "filestream.h"
//日志等级

#define LOG_DEBUG DefLog.Debug()
#define LOG_INFO  DefLog.Info()
#define LOG_WARN  DefLog.Warn()
#define LOG_ERROR DefLog.Error()
#define LOG_FATAL DefLog.Fatal()

#define DEBUG(...) DefLog.log("DEBUG",__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)
#define INFO(...)  DefLog.log("INFO",__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)
#define WARN(...)  DefLog.log("WARN",__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)
#define ERROR(...) DefLog.log("ERROR",__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)
#define FATAL(...) DefLog.log("FATAL",__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)

class Log
{
public:
	Log(std::string& log_path = std::string("./log.log"),int32_t level=0,int32_t type=0);
	~Log();

	FileStream& logstream();
	void debug(const char* format, ...);
	void info(const char* format, ...);
	void warn(const char* format, ...);
	void error(const char* format, ...);
	void fatal(const char* format, ...);
	//额外信息
	void log(const char* level, const char* pszFileName, const char* pszFuncName, int32_t nLineNum, const char* format, ...);

private:
	void _log(const char* level, const char* format, va_list argptr);
	void _log(const char* level, const char* pszFileName, const char* pszFuncName, int32_t nLineNum, const char* format, va_list argptr);

private:
	int32_t m_level;
	int32_t m_type;

	FileStream m_file_stream;

public:
	//流输入
	LogFileStream Debug() { return LogFileStream(m_file_stream,"DEBUG"); }
	LogFileStream Info() { return LogFileStream(m_file_stream, "INFO"); }
	LogFileStream Warn() { return LogFileStream(m_file_stream, "WARN"); }
	LogFileStream Error() { return LogFileStream(m_file_stream, "ERROR"); }
	LogFileStream Fatal() { return LogFileStream(m_file_stream, "FATAL"); }
};

static Log DefLog;



