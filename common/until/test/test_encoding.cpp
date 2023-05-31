#include <gtest/gtest.h>
#include "encoding.h"
#include <winsock2.h>
using namespace std;
#pragma comment(lib, "ws2_32.lib")  // 链接 ws2_32.lib 库

bool IsBigEndian() {
	// 创建一个16位整数（2个字节）并初始化为1
	uint16_t value = 0x0001;

	// 将16位整数的内存表示转换为字符数组
	uint8_t* bytes = reinterpret_cast<uint8_t*>(&value);

	// 如果低字节存储在内存的低地址，则为小端序；否则为大端序
	return (bytes[0] == 0x00);
}

TEST(testcase0, htonl)
{
	uint32_t value = 123456789;  // 假设原始值为当前机器字节序表示的整数
	std::cout << "Original value: " << value << std::endl;

	uint32_t convertedValue = htonl(value);
	std::cout << "Converted value (Network Byte Order): " << convertedValue << std::endl;

	uint32_t convertedValue2 = HostToBigEndian((int32_t)value);
	std::cout << "Converted value2 (Network Byte Order): " << convertedValue2 << std::endl;

	uint32_t convertedbackValue = ntohl(value);
	std::cout << "Converted back value (Network Byte Order): " << convertedValue << std::endl;
	
	uint32_t convertedbackValue2 = BigEndianToHost((int32_t)value);
	std::cout << "Converted value2 (Network Byte Order): " << convertedbackValue2 << std::endl;
	
	EXPECT_EQ(convertedValue, convertedValue2);
	EXPECT_EQ(convertedbackValue, convertedbackValue2);
}
TEST(testcase1, encoding)
{
	if (IsBigEndian()) {
		std::cout << "Current host is big endian" << std::endl;
	}
	else {
		std::cout << "Current host is little endian" << std::endl;
	}

	int16_t value = 0x1122;  // 输入值
	std::cout << "Current byte order: " << std::hex << value << std::endl;

	// 将小端序转换为主机字节序
	int16_t hostValue = BigEndianToHost(value);

	// 打印转换结果
	std::cout << "Host byte order: " << std::hex << hostValue << std::endl;

	// 将主机字节序转换为大端序
	int16_t bigEndianValue = HostToBigEndian(hostValue);

	// 打印转换结果
	std::cout << "Big endian order: " << std::hex << bigEndianValue << std::endl;
}
TEST(testcase2, encoding_int64)
{
	int64_t originalValue = 0x8877665544332211LL;

	// 测试 hostToBigEndian 函数
	int64_t convertedValue = HostToBigEndian(originalValue);
	std::cout << "Converted value (Host to BigEndian): " << originalValue << std::endl;

	// 测试 bigEndianToHost 函数
	int64_t convertedBackValue = BigEndianToHost(convertedValue);
	std::cout << "Converted back value (BigEndian to Host): " << convertedValue << std::endl;

	EXPECT_EQ(originalValue, convertedBackValue);

	// 检查转换前后的值是否相等
	if (originalValue == convertedBackValue) {
		std::cout << "Endian conversion successful." << std::endl;
	}
	else {
		std::cout << "Endian conversion failed." << std::endl;
	}
}
TEST(testcase3, encoding_int32)
{
	int32_t originalValue = 0x12345678;
	std::cout << "Original value (Little Endian): " << originalValue << std::endl;

	int32_t bigEndianValue = HostToBigEndian(originalValue);
	std::cout << "Converted value (Big Endian): " << bigEndianValue << std::endl;

	int32_t hostValue = BigEndianToHost(bigEndianValue);
	std::cout << "Converted back value (Little Endian): " << hostValue << std::endl;

	if (originalValue == hostValue) {
		std::cout << "Conversion Successful" << std::endl;
	}
	else {
		std::cout << "Conversion Failed" << std::endl;
	}

	EXPECT_EQ(originalValue, hostValue);
}
TEST(testcase4, encoding_int16)
{
	int16_t originalValue = 0x1122;  // 假设原始值为小端序表示的整数

	int16_t convertedValue = BigEndianToHost(originalValue);
	int16_t convertedBackValue = HostToBigEndian(convertedValue);

	std::cout << "Original value (Little Endian): " << originalValue << std::endl;
	std::cout << "Converted value (Big Endian): " << convertedValue << std::endl;

	if (originalValue == convertedBackValue) {
		std::cout << "Endian conversion successful." << std::endl;
	}
	else {
		std::cout << "Endian conversion failed." << std::endl;
	}

	EXPECT_EQ(originalValue, convertedBackValue);
}

