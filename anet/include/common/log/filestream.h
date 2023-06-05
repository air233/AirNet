#pragma once

#include <stdio.h>
#include<sstream>
#include<string>
#include <mutex>

enum cutMode 
{
	HOUR,
	DAY,
	MIN,
};

class FileStream
{
public:
	static std::string getLogFileName(const std::string& base, uint8_t mode, time_t* now);

public:
	FileStream(std::string path, std::string file_name, int32_t level, uint8_t cutm, uint8_t async);
	~FileStream();

	bool init();
	void update(time_t update);
	void flush();
	void log(const char* level, const char* log);
	std::ostringstream& stream();
	
	void set_dirty();
private:
	/*�ָ�����*/
	int32_t m_level;
	uint8_t m_cut_mode;
	uint8_t m_async;

	/*��־�и�ʱ��*/
	int32_t m_roll;

	/*��־ˢ��ʱ��*/
	time_t m_flush;
	uint8_t m_dirty;

	/*��־����*/
	std::string m_path;
	std::string m_base;
	std::string m_log_name;
	FILE* m_file;
	std::mutex m_file_mutex;

	std::ostringstream m_oss;
};


class LogFileStream
{
public:
	LogFileStream(FileStream& log, const char* level);
	~LogFileStream();

	template<typename T>
	std::ostringstream& operator << (const T& obj);

private:
	FileStream& m_log;
	std::string m_level;
};

template<typename T>
std::ostringstream& LogFileStream::operator <<(const T& obj)
{
	m_log.stream() << obj;

	return m_log.stream();
}
