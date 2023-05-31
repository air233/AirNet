#include <glog/logging.h>

int main(int argc, char* argv[])
{
	google::InitGoogleLogging("glog");

	LOG(INFO) << "test";

	return 0;
}