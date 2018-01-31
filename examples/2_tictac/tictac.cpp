#include <modulight/application.hpp>

using namespace modulight::user_interface;

int main(int argc, char ** argv)
{
	modulight::Application app(argc, argv);

	Process * tic = app.addProcess("2_tic");
	Process * tac = app.addProcess("2_tac");

	app.connect(tic, "out", tac, "in");

	app.start();
	return 0;
}