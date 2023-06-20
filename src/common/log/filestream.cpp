#include "filestream.h"
#include "../until/times.h"
#include<assert.h>
#include <iostream>
#include <filesystem>

#define FILESIZE 1024

#ifdef _MSC_VER
#include <windows.h>
#else
#include <sys/stat.h>
#endif


bool createDirectoryIfNotExists(const std::string& directoryPath) {
#ifdef _MSC_VER
	if (!CreateDirectory(directoryPath.c_str(), NULL)) {
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			//std::cout << "Directory already exists: " << directoryPath << std::endl;
			return true;
		}
		else {
			std::cout << "Failed to create directory: " << directoryPath << std::endl;
			return false;
		}
	}
	//std::cout << "Directory created: " << directoryPath << std::endl;
	return true;
#else
	struct stat st;
	if (stat(directoryPath.c_str(), &st) == 0) {
		if (S_ISDIR(st.st_mode)) {
			std::cout << "Directory already exists: " << directoryPath << std::endl;
			return true;
		}
	}

	if (mkdir(directoryPath.c_str(), 0777) == 0) {
		std::cout << "Directory created: " << directoryPath << std::endl;
		return true;
	}

	std::cout << "Failed to create directory: " << directoryPath << std::endl;
	return false;
#endif
}


std::string FileStream::getLogFileName(const std::string& basename, uint8_t mode, time_t* now)
{
	std::string filename;
	filename.reserve(basename.size() + 64);
	filename = basename;

	*now = GetTime();

	std::tm tm;
	localtime_s(&tm, now);

	char timebuf[32] = { 0 };
	if (mode == HOUR)
	{
		std::strftime(timebuf, sizeof(timebuf), "%Y_%m_%d_%H", &tm);
	}
	else if(mode == DAY)
	{
		std::strftime(timebuf, sizeof(timebuf), "%Y_%m_%d", &tm);
	}
	else
	{
		std::strftime(timebuf, sizeof(timebuf), "%Y_%m_%d_%H_%M", &tm);
	}

	filename = filename + "_" + timebuf;
	filename += ".log";

	return filename;
}

FileStream::FileStream(std::string path, std::string file_name, int32_t level, uint8_t cutm, uint8_t async) :
	m_file(nullptr), m_level(level), m_cut_mode(cutm), m_async(async), m_roll(GetTime()),m_flush(GetMSTime())
{
	time_t now_time;

	m_path = path;
	
	createDirectoryIfNotExists(m_path);

	m_base = file_name;

	m_log_name = m_path + FileStream::getLogFileName(m_base, m_cut_mode, &now_time);
	
	init();
}

FileStream::~FileStream()
{
	if(m_file)
		fclose(m_file);
}

bool FileStream::init()
{
	//std::cout << m_log_name << std::endl;

	if (m_file)
		fclose(m_file);

	m_file = fopen(m_log_name.c_str(), "a");

	if (m_file == nullptr)
	{
		return false;
	}

	return true;
}

void FileStream::update(time_t update)
{
	if (m_async == 0) return;

	time_t now = GetMSTime();

	if (((now - m_flush) > 30 && m_dirty == 1) || (m_oss.tellp() > FILESIZE))
	{
		std::lock_guard<std::mutex> lock(m_file_mutex);

		flush();

		m_flush = now;
	}
}

void FileStream::flush()
{
	std::ostringstream out;
	out << m_oss.str() << "\n";
	m_oss.str("");
	m_oss.clear();

	time_t now;
	std::string log_name = m_path + FileStream::getLogFileName(m_base, m_cut_mode, &now);
	if (log_name != m_log_name)
	{
		assert(this->init() != false);
	}

	fprintf(m_file, out.str().c_str());
	fflush(m_file);

	m_dirty = 0;
}

void FileStream::log(const char* level, const char* log)
{
	m_dirty = 1;
	m_oss << "["<<GetMSTimeStr()<<"]" << "[" << level << "]" << " " << log << "\n";
	//std::cout << m_oss.str() << std::endl;

	if(m_async == 0)
		flush();
}

std::ostringstream& FileStream::stream()
{
	return m_oss;
}

void FileStream::set_dirty()
{
	m_dirty = 1;
}

//================================================================

//TODO:多线程安全读写
LogFileStream::LogFileStream(FileStream& log, const char* level)
:m_log(log), m_level(level)
{
	m_oss << "[" << GetMSTimeStr() << "]" << "[" << level << "] ";
}

LogFileStream::LogFileStream(const LogFileStream& file):
	m_log(file.m_log), m_level(file.m_level)
{
	m_oss << file.m_oss.str() << "\n";
}

LogFileStream::~LogFileStream()
{
	m_log.stream() << m_oss.str();
	m_log.flush();

	std::cout << m_oss.str() << std::endl;

	if (m_level == "FATAL")
	{
		abort();
	}
}
