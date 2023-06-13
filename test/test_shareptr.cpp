#include <memory>
#include <iostream>

using namespace std;
class CC
{
public:
	CC() {};
	~CC()
	{
		std::cout << "~CC" << endl;
	}
};

shared_ptr<CC> makeCC()
{
	return shared_ptr<CC>(new CC);
}

int main()
{
	{
		shared_ptr<CC> pc = makeCC();
	}

	return 0;
}