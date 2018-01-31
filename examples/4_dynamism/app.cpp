#include <modulight/application.hpp>

using namespace modulight::user_interface;

int main(int argc, char ** argv)
{
	modulight::Application app(argc, argv);

	Process * dynamicMaster = app.addProcess("4_dynamic_master");
	app.allowProcessToAlterNetwork(dynamicMaster);

	app.start();
	return 0;
}