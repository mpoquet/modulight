#include <modulight/application.hpp>

using namespace modulight::user_interface;

int main(int argc, char ** argv)
{
	modulight::Application app(argc, argv);

	ParallelProcess * parallelProcess = app.addParallelProcess("6_parallel_module", 4);
	Process * generator = app.addProcess("6_generator");

	Process * p = parallelProcess->process(0);
	app.connect(generator, "out", p, "in");

	app.start();
	return 0;
}