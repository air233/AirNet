#include <string>
#include <iostream>
#include <vector>

const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

std::string base64_encode(const std::string& input) 
{
	std::string encoded;
	int i = 0, j = 0;
	unsigned char char_array_3[3], char_array_4[4];

	for (const auto& c : input) 
	{
		char_array_3[i++] = c;

		if (i == 3) 
		{
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; i < 4; i++) 
			{
				encoded += base64_chars[char_array_4[i]];
			}
			i = 0;
		}
	}

	if (i != 0) 
	{
		for (j = i; j < 3; j++) 
		{
			char_array_3[j] = '\0';
		}

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; j < i + 1; j++) 
		{
			encoded += base64_chars[char_array_4[j]];
		}

		while (i++ < 3) 
		{
			encoded += '=';
		}
	}

	return encoded;
}

std::string base64_decode(const std::string& encoded) 
{
	int i = 0, j = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string decoded;

	for (const auto& c : encoded) 
	{
		if (c == '=') 
		{
			break;
		}

		char_array_4[i++] = c;
		if (i == 4) 
		{
			for (i = 0; i < 4; i++) 
			{
				char_array_4[i] = (unsigned char)base64_chars.find(char_array_4[i]);
			}

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; i < 3; i++) 
			{
				decoded += char_array_3[i];
			}
			i = 0;
		}
	}

	if (i != 0) 
	{
		for (j = i; j < 4; j++) 
		{
			char_array_4[j] = 0;
		}

		for (j = 0; j < 4; j++) 
		{
			char_array_4[j] = (unsigned char)base64_chars.find(char_array_4[j]);
		}

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; j < i - 1; j++) 
		{
			decoded += char_array_3[j];
		}
	}

	return decoded;
}