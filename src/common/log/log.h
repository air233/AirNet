#pragma once
#include<string>
#include <stdio.h>
#include<sstream>

#include "logstream.h"
//��־�ȼ�

///*����*/
//#define LOG_DEBUG 
///*��Ϣ*/
//#define LOG_INFO
///*����*/
//#define LOG_WARN
///*����*/
//#define LOG_ERROR
///*����*/
//#define LOG_FATAL


//LOG_DEBUG << "hello world"


class Log
{
public:
	Log(std::string& log_path = std::string("./log.log"));
	~Log();

	void flush();

	std::ostringstream& strem();

	LogStream& logstream();

	void debug(std::string log);



private:
	//int m_level;

	FILE* m_file;

	std::ostringstream m_oss;

	LogStream m_stream;

public:
	LogOutStream DEBUG;
	LogOutStream INFO;
	LogOutStream WRAN;
	LogOutStream ERROR;
	LogOutStream FATAL;
};

//static Log log;



