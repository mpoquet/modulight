#include <iostream>
#include <unistd.h>
#include <modulight/module.hpp>

using namespace std;
using namespace modulight;

int main(int argc, char ** argv)
{
	Module tic("DynamicTic", argc, argv);
	tic.addOutputPort("out");

	if (!tic.initialize())
		return 0;

	int delay = tic.arguments().getInt("minimumDelay", 0);

	for (int i = 0; tic.isRunning(); ++i)
	{
		tic.wait();

		if (delay > 0)
			usleep(delay * 1000);

		tic.send("out", (char*)&i, sizeof(int));
	}

	return 0;
}