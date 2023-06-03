#include <gtest/gtest.h>
#include "buffer.h"
#include <string>
using namespace std;

string test_buff1()
{
	Buffer buff;

	buff.push("hello", 5);

	return buff.get(5);
}

TEST(testcase1, test_expect)
{
	Buffer buff;
	buff.push("hello", 5);
	EXPECT_EQ(buff.get(4), string("hell"));
	EXPECT_EQ(buff.get(1), string("o"));
	EXPECT_EQ(buff.get(1), string(""));

	double x = 3.152132;
	buff.push(x);

	double y = 0;
	if (true == buff.peek(y))
	{
		cout << y << endl;
	}
}

TEST(testcase2, test_expect)
{
	EXPECT_EQ(test_buff1(), string("hello"));
}

TEST(testcase3, int)
{
	Buffer buff;

	int8_t in8 = -120;
	buff.pushInt8(in8);
	int8_t in81 = 0;
	if (false == buff.peekInt8(in81))
	{
		cout << "int8 false" << endl;
	}
	EXPECT_EQ(in8, in81);

	uint8_t uin8 = 255;
	buff.pushUint8(uin8);
	uint8_t uin81 = 0;
	if (false == buff.peekUint8(uin81))
	{
		cout << "uint8 false" << endl;
	}
	EXPECT_EQ(uin8, uin81);


	int16_t in16 = -500;
	buff.pushInt16(in16);
	int16_t in161 = 0;
	if (false == buff.peekInt16(in161))
	{
		cout << "int16 false" << endl;
	}
	//cout << (int)in16 << endl;
	//cout << (int)in161 << endl;
	EXPECT_EQ(in16, in161);

	uint16_t uin16 = 500;
	buff.pushUint16(uin16);
	uint16_t uin161 = 0;
	if (false == buff.peekUint16(uin161))
	{
		cout << "uint16 false" << endl;
	}
	EXPECT_EQ(uin16, uin161);


	int32_t in32 = -55555;
	buff.pushInt32(in32);
	int32_t in321 = 0;
	if (false == buff.peekInt32(in321))
	{
		cout << "int32 false" << endl;
	}
	EXPECT_EQ(in32, in321);

	uint32_t uin32 = 55555;
	buff.pushUint32(uin32);
	uint32_t uin321 = 0;
	if (false == buff.peekUint32(uin321))
	{
		cout << "uint32 false" << endl;
	}
	EXPECT_EQ(uin32, uin321);


	int64_t in64 = -0x8877665544332211LL;
	buff.pushInt64(in64);
	int64_t in641 = 0;
	if (false == buff.peekInt64(in641))
	{
		cout << "int64 false" << endl;
	}
	EXPECT_EQ(in64, in641);

	uint64_t uin64 = 0x8877665544332211LL;
	buff.pushUint64(uin64);
	uint64_t uin641 = 0;
	if (false == buff.peekUint64(uin641))
	{
		cout << "uint64 false" << endl;
	}
	EXPECT_EQ(uin64, uin641);

}

TEST(testcase4, swap)
{
	Buffer buff;
	Buffer buff1;
	buff.push("hello", 5);
	buff.swap(buff1);

	EXPECT_EQ(buff.get(5), "");
	EXPECT_EQ(buff1.get(5),"hello");
}

TEST(testcase5, string)
{
	Buffer buff;

	std::string str("hello world");
	buff.pushString(str);

	size_t size = 0;
	buff.peek(size);

	std::string newstr;
	buff.peekString(newstr, size);

	EXPECT_EQ(newstr, str);
	EXPECT_EQ(buff.get(1), "");
}

TEST(testcase6, cstring)
{
	Buffer buff;

	const char* pst = "hello world";
	//cout << strlen(pst);
	buff.pushCString(pst, strlen(pst));

	size_t size = 0;
	buff.peek(size);

	std::string newstr;
	buff.peekString(newstr, size);

	EXPECT_EQ(newstr, pst);
	EXPECT_EQ(buff.get(1), "");

	buff.pushInt32(12);
	int32_t in32 = 0;
	buff.peekInt32(in32);
	EXPECT_EQ(in32, 12);
}