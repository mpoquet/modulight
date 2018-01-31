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

	int msgCount = tic.arguments().getInt("sendCount", 42);
	for (int i = 0; i < msgCount; ++i)
	{
		wait();
		tic.send("out", (char*)&i, sizeof(int));
	}

	return 0;
}