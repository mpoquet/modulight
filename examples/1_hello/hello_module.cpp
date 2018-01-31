#include <iostream>
#include <modulight/module.hpp>

using namespace std;
using namespace modulight;

int main(int argc, char ** argv)
{
	Module hello("Hello", argc, argv);

	if (!hello.initialize())
		return 0;

	hello.display() << "Hello world!" << endl;

	return 0;
}