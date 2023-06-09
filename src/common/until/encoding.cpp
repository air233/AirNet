#include "encoding.h"

int64_t BigEndianToHost(int64_t value)
{
#ifdef _MSC_VER
	if (_WIN32)
	{
		value = ((value & 0xFF00000000000000) >> 56) |
			((value & 0x00FF000000000000) >> 40) |
			((value & 0x0000FF0000000000) >> 24) |
			((value & 0x000000FF00000000) >> 8) |
			((value & 0x00000000FF000000) << 8) |
			((value & 0x0000000000FF0000) << 24) |
			((value & 0x000000000000FF00) << 40) |
			((value & 0x00000000000000FF) << 56);
	}
#else
	if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
	{
		value = ((value & 0xFF00000000000000) >> 56) |
			((value & 0x00FF000000000000) >> 40) |
			((value & 0x0000FF0000000000) >> 24) |
			((value & 0x000000FF00000000) >> 8) |
			((value & 0x00000000FF000000) << 8) |
			((value & 0x0000000000FF0000) << 24) |
			((value & 0x000000000000FF00) << 40) |
			((value & 0x00000000000000FF) << 56);
	}
#endif
	return value;
}

uint64_t BigEndianToHost(uint64_t value)
{
	return (uint64_t)BigEndianToHost((int64_t)value);
}

uint32_t BigEndianToHost(uint32_t value)
{
	return (uint32_t)BigEndianToHost((int32_t)value);
}

uint16_t BigEndianToHost(uint16_t value)
{
	return (uint16_t)BigEndianToHost((int16_t)value);
}

int64_t HostToBigEndian(int64_t value)
{
#ifdef _MSC_VER
	if (_WIN32)
	{
		value = ((value & 0xFF00000000000000) >> 56) |
			((value & 0x00FF000000000000) >> 40) |
			((value & 0x0000FF0000000000) >> 24) |
			((value & 0x000000FF00000000) >> 8) |
			((value & 0x00000000FF000000) << 8) |
			((value & 0x0000000000FF0000) << 24) |
			((value & 0x000000000000FF00) << 40) |
			((value & 0x00000000000000FF) << 56);
	}
#else
	if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
	{
		value = ((value & 0xFF00000000000000) >> 56) |
			((value & 0x00FF000000000000) >> 40) |
			((value & 0x0000FF0000000000) >> 24) |
			((value & 0x000000FF00000000) >> 8) |
			((value & 0x00000000FF000000) << 8) |
			((value & 0x0000000000FF0000) << 24) |
			((value & 0x000000000000FF00) << 40) |
			((value & 0x00000000000000FF) << 56);
	}
#endif
	return value;
}

uint64_t HostToBigEndian(uint64_t value)
{
	return (uint64_t)HostToBigEndian((int64_t)value);
}

uint32_t HostToBigEndian(uint32_t value)
{
	return (uint32_t)HostToBigEndian((int32_t)value);
}

uint16_t HostToBigEndian(uint16_t value)
{
	return (uint16_t)HostToBigEndian((int16_t)value);
}

int32_t BigEndianToHost(int32_t value)
{
#ifdef _MSC_VER
	if (_WIN32)
	{
		value = ((value & 0xFF000000) >> 24) |
			((value & 0x00FF0000) >> 8) |
			((value & 0x0000FF00) << 8) |
			((value & 0x000000FF) << 24);
	}
#else
	if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
	{
		value = ((value & 0xFF000000) >> 24) |
			((value & 0x00FF0000) >> 8) |
			((value & 0x0000FF00) << 8) |
			((value & 0x000000FF) << 24);
	}
#endif
	return value;
}

int32_t HostToBigEndian(int32_t value)
{
#ifdef _MSC_VER
	if (_WIN32)
	{
		value = ((value & 0xFF000000) >> 24) |
			((value & 0x00FF0000) >> 8) |
			((value & 0x0000FF00) << 8) |
			((value & 0x000000FF) << 24);
	}
#else
	if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
	{
		value = ((value & 0xFF000000) >> 24) |
			((value & 0x00FF0000) >> 8) |
			((value & 0x0000FF00) << 8) |
			((value & 0x000000FF) << 24);
	}
#endif
	return value;
}

int16_t BigEndianToHost(int16_t value)
{
#ifdef _MSC_VER
	if (_WIN32)
	{
		value = ((value & 0xFF) << 8) | ((value & 0xFF00) >> 8);
	}
#else
	if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
	{
		value = ((value & 0xFF) << 8) | ((value & 0xFF00) >> 8);
	}
#endif
	return value;
}

int16_t HostToBigEndian(int16_t value)
{
#ifdef _MSC_VER
	if (_WIN32)
	{
		value = ((value & 0xFF) << 8) | ((value & 0xFF00) >> 8);
	}
#else
	if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
	{
		value = ((value & 0xFF) << 8) | ((value & 0xFF00) >> 8);
	}
#endif
	return value;
}
