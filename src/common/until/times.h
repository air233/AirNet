#pragma once
#include <cstdint>
#include <string>

uint32_t GetTime();
uint64_t GetMSTime();

std::string GetTimeStr(uint32_t time=0);
std::string GetMSTimeStr(uint64_t mstime = 0);

uint32_t GetWeekDay();
uint32_t GetWeekDay(uint32_t time);