#pragma once
#include<string>
#include <stdio.h>
#include<sstream>

#include "filestream.h"
//日志等级

#define DEFAULTPATH "./log/"
#define DEFAULTNAME "log"

#define LOG_DEBUG default_log.Debug() << "(" << __FUNCTION__ << ") "
#define LOG_INFO  default_log.Info() << "(" << __FUNCTION__ << ") "
#define LOG_WARN  default_log.Warn() << "(" << __FUNCTION__ << ") "
#define LOG_ERROR default_log.Error() << "(" << __FUNCTION__ << ") "
#define LOG_FATAL default_log.Fatal() << "(" << __FUNCTION__ << ") "


#define DEBUG(...) default_log.log("DEBUG",__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)
#define INFO(...)  default_log.log("INFO",__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)
#define WARN(...)  default_log.log("WARN",__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)
#define ERROR(...) default_log.log("ERROR",__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)
#define FATAL(...) default_log.log("FATAL",__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)

//TODO:未使用
#define FUNCNAME(type) type |= 1;
#define LINENUM(type) type |= (1<<1);
#define FILENAME(type) type |= (1<<2);

class Log
{
public:
	Log(const std::string& log_path = std::string("./log/"),const std::string& file_name = std::string("log.log"), int32_t level=0,int32_t type=0);
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
	LogFileStream Info()  { return LogFileStream(m_file_stream, "INFO"); }
	LogFileStream Warn()  { return LogFileStream(m_file_stream, "WARN"); }
	LogFileStream Error() { return LogFileStream(m_file_stream, "ERROR"); }
	LogFileStream Fatal() { return LogFileStream(m_file_stream, "FATAL"); }
};

static Log default_log(DEFAULTPATH,DEFAULTNAME);



