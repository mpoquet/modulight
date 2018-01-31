#include <iostream>
#include <modulight/module.hpp>

using namespace std;
using namespace modulight;

int main(int argc, char ** argv)
{
	Module tac("Tac", argc, argv);
	tac.addInputPort("in");

	if (!tac.initialize())
		return 0;

	int value;
	for (int i = 0; i < 42; ++i)
	{
		tac.wait("in");

		if (tac.readMessage("in", (char*)&value, sizeof(int)))
			tac.display() << "received " << value << endl;
	}

	return 0;
}