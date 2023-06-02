#include <gtest/gtest.h>
#include "encoding.h"
#include <winsock2.h>
using namespace std;
#pragma comment(lib, "ws2_32.lib")  // ���� ws2_32.lib ��

bool IsBigEndian() {
	// ����һ��16λ������2���ֽڣ�����ʼ��Ϊ1
	uint16_t value = 0x0001;

	// ��16λ�������ڴ��ʾת��Ϊ�ַ�����
	uint8_t* bytes = reinterpret_cast<uint8_t*>(&value);

	// ������ֽڴ洢���ڴ�ĵ͵�ַ����ΪС���򣻷���Ϊ�����
	return (bytes[0] == 0x00);
}

TEST(testcase0, htonl)
{
	uint32_t value = 123456789;  // ����ԭʼֵΪ��ǰ�����ֽ����ʾ������
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

	int16_t value = 0x1122;  // ����ֵ
	std::cout << "Current byte order: " << std::hex << value << std::endl;

	// ��С����ת��Ϊ�����ֽ���
	int16_t hostValue = BigEndianToHost(value);

	// ��ӡת�����
	std::cout << "Host byte order: " << std::hex << hostValue << std::endl;

	// �������ֽ���ת��Ϊ�����
	int16_t bigEndianValue = HostToBigEndian(hostValue);

	// ��ӡת�����
	std::cout << "Big endian order: " << std::hex << bigEndianValue << std::endl;
}
TEST(testcase2, encoding_int64)
{
	int64_t originalValue = 0x8877665544332211LL;

	// ���� hostToBigEndian ����
	int64_t convertedValue = HostToBigEndian(originalValue);
	std::cout << "Converted value (Host to BigEndian): " << originalValue << std::endl;

	// ���� bigEndianToHost ����
	int64_t convertedBackValue = BigEndianToHost(convertedValue);
	std::cout << "Converted back value (BigEndian to Host): " << convertedValue << std::endl;

	EXPECT_EQ(originalValue, convertedBackValue);

	// ���ת��ǰ���ֵ�Ƿ����
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
	int16_t originalValue = 0x1122;  // ����ԭʼֵΪС�����ʾ������

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

