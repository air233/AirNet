#pragma once
#include <cstdint>

/*大端序(网络序)与主机序互转*/

int64_t BigEndianToHost(int64_t value);
int64_t HostToBigEndian(int64_t value);

uint64_t BigEndianToHost(uint64_t value);
uint64_t HostToBigEndian(uint64_t value);

int32_t BigEndianToHost(int32_t value);
int32_t HostToBigEndian(int32_t value);

uint32_t BigEndianToHost(uint32_t value);
uint32_t HostToBigEndian(uint32_t value);

int16_t BigEndianToHost(int16_t value);
int16_t HostToBigEndian(int16_t value);

uint16_t BigEndianToHost(uint16_t value);
uint16_t HostToBigEndian(uint16_t value);