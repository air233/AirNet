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
	std::string newstr;
	buff.peekString(newstr, str.size());

	EXPECT_EQ(newstr, str);
	EXPECT_EQ(buff.get(1), "");
}

TEST(testcase6, cstring)
{
	Buffer buff;

	const char* pst = "hello world";
	buff.pushUint32(strlen(pst));
	buff.pushCString(pst, strlen(pst));

	uint32_t size = 0;
	std::string newstr;
	buff.peekUint32(size);
	int newstr_len = buff.peekString(newstr, size);
	std::cout << "size:" << size << std::endl;
	std::cout << "newstr_len:" << newstr_len << std::endl;

	EXPECT_EQ(newstr, pst);
	EXPECT_EQ(buff.get(1), "");
	EXPECT_EQ(buff.size(), 0);

	buff.pushCString(pst, strlen(pst));
	char data[1024] = { 0 };
	size_t data_len = buff.peekCString(data, 1024);
	std::cout << "data:" << data << std::endl;
	std::cout << "data_len:" << data_len << std::endl;
	EXPECT_EQ(std::string(data),pst);


	buff.pushInt32(12);
	int32_t in32 = 0;
	buff.peekInt32(in32);
	EXPECT_EQ(in32, 12);

	std::cout << "size_t len:" << sizeof size_t << std::endl;
	std::cout << "unsigned int len:" << sizeof (unsigned int) << std::endl;
	std::cout << "unsigned __int64 len:" << sizeof (unsigned __int64) << std::endl;
}

TEST(testcase7, cstring2)
{
	Buffer buff;

	const char* pst = "hello world";

	buff.pushString(std::string("hhhhh"));

	buff.insertCString(pst, 0, 11);
	std::string newstr;
	int newstr_len = buff.peekString(newstr, 11);
	EXPECT_EQ(newstr, pst);
	std::cout << "newstr:" << newstr << std::endl;

	newstr_len = buff.peekString(newstr, buff.size());
	EXPECT_EQ(newstr, "hhhhh");
	std::cout << "newstr:" << newstr << std::endl;
}


TEST(testcase8, buff)
{
	Buffer buff;
	EXPECT_EQ(buff.readableSize(), 0);
	EXPECT_EQ(buff.writableSize(), kInitSize);

	const string str(200, 'x');
	buff.pushString(str);
	EXPECT_EQ(buff.readableSize(), str.size());
	EXPECT_EQ(buff.writableSize(), kInitSize - str.size());

	string str2;
	buff.peekString(str2, 50);
	EXPECT_EQ(str2.size(), 50);
	EXPECT_EQ(buff.readableSize(), str.size() - str2.size());

}

TEST(testcase9, buffGrow)
{
	Buffer buf;
	buf.pushString(string(400, 'y'));
	EXPECT_EQ(buf.readableSize(), 400);
	EXPECT_EQ(buf.writableSize(), kInitSize - 400);

	string str;
	buf.peekString(str,50);
	EXPECT_EQ(buf.readableSize(), 350);
	EXPECT_EQ(buf.writableSize(), kInitSize - 400);

	buf.pushString(string(1000, 'z'));
	EXPECT_EQ(buf.readableSize(), 1350);
	EXPECT_EQ(buf.writableSize(), 0);
	std::cout << "writableSize:"<< buf.writableSize() << std::endl;

}