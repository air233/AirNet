#include <iostream>
#include <gtest/gtest.h>
#include <vector>
using namespace std;

int main(int argc, char** argv)
{
	std::cout << "run buffer test --> " << std::endl << std::endl;

	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}



//int main(int argc, char** argv)
//{
//	int array[] = { 1,2,3,4,5,6 };
//	vector<int> v1(5);
//	cout << "v1.size:" << v1.size() << endl;
//	cout << "v1.capacity:" << v1.capacity() << endl;
//	memcpy(v1.data(), array, 1);
//	for (int v : v1)
//	{
//		cout << "v:" << v << endl;
//	}
//	cout << "v1.size:" << v1.size() << endl;
//	cout << "v1.capacity:" << v1.capacity() << endl;
//
//	v1.resize(1300);
//	cout << "v1.size:" << v1.size() << endl;
//	cout << "v1.capacity:" << v1.capacity() << endl;
//}


