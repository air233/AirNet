#pragma once
#include <cstdint>

/*大端序与主机序互转*/

int64_t BigEndianToHost(int64_t value);
int64_t HostToBigEndian(int64_t value);

int32_t BigEndianToHost(int32_t value);
int32_t HostToBigEndian(int32_t value);

int16_t BigEndianToHost(int16_t value);
int16_t HostToBigEndian(int16_t value);
