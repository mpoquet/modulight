#include <iostream>
#include <unistd.h>
#include <modulight/module.hpp>

using namespace std;
using namespace modulight;

int main(int argc, char ** argv)
{
	Module tac("DynamicTac", argc, argv);
	tac.addInputPort("in");

	if (!tac.initialize())
		return 0;

	int value;
	while (tac.isRunning())
	{
		tac.wait("in");

		if (tac.readMessage("in", (char*)&value, sizeof(int)))
			tac.display() << "received " << value << endl;
	}

	return 0;
}