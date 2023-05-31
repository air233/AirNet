#include "md5.h"
#include <sstream>
#include <iomanip>
#include <openssl/md5.h>

std::string calculateMD5(const std::string& input)
{
	MD5_CTX context;
	MD5_Init(&context);
	
	MD5_Update(&context, input.c_str(), input.length());
	
	unsigned char digest[MD5_DIGEST_LENGTH];
	MD5_Final(digest, &context);

	std::stringstream ss;
	for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
		ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
	}

	return ss.str();
}

