#include <iostream>
#include <unistd.h>
#include <modulight/module.hpp>

using namespace std;
using namespace modulight;

int main(int argc, char ** argv)
{
	Module m("DynamicMaster", argc, argv);

	if (!m.initialize())
		return 0;

	ArgumentWriter writerTic;
	writerTic.addInt("minimumDelay", 1000);

	m.spawnProcess("4_dynamic_tic", writerTic, "127.0.0.1");
	m.spawnProcess("4_dynamic_tac");
	m.addConnection("DynamicTic", 0, "out", "DynamicTac", 0, "in");
	sleep(5);
	m.killProcess("DynamicTic", 0);

	// Let's create DynamicTic again, but without speed limitation
	m.spawnProcess("4_dynamic_tic");
	m.addConnection("DynamicTic", 1, "out", "DynamicTac", 0, "in", true); // Instance number of tic is now 1 ; the connection is lossy
	usleep(20 * 1000);
	m.killProcess("DynamicTic", 1);
	m.killProcess("DynamicTac", 0);

	return 0;
}