#include <iostream>
#include <modulight/module.hpp>

using namespace std;
using namespace modulight;

int main(int argc, char ** argv)
{
	Module tic("Tic", argc, argv);
	tic.addOutputPort("out");

	if (!tic.initialize())
		return 0;

	for (int i = 0; i < 42; ++i)
	{
		wait();
		tic.send("out", (char*)&i, sizeof(int));
	}

	return 0;
}