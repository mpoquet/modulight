#include <modulight/application.hpp>

using namespace modulight::user_interface;
using namespace modulight;

int main(int argc, char ** argv)
{
	Application hello(argc, argv);
	hello.addProcess("1_helloModule");

	hello.start();
	return 0;
}