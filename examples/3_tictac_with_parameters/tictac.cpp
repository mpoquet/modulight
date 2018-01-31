#include <modulight/application.hpp>

using namespace modulight::user_interface;

int main(int argc, char ** argv)
{
	modulight::Application app(argc, argv);

	Process * tic = app.addProcess("3_tic");
	Process * tac = app.addProcess("3_tac");

	app.connect(tic, "out", tac, "in");

	int msgCount = app.arguments().getInt("msgCount", 42);
	app.setArgument(tic, "sendCount", msgCount);
	app.setArgument(tac, "recvCount", msgCount);

	app.start();
	return 0;
}