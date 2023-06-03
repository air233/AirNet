#include <glog/logging.h>

int main(int argc, char* argv[])
{
	//google::InitGoogleLogging("glog");

	LOG(INFO) << "test:" << 123321;

	LOG(INFO) << "999999";

	LOG(INFO) << "666666";

	return 0;
}