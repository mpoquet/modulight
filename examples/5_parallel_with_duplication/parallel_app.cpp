#include <modulight/application.hpp>

using namespace modulight::user_interface;

int main(int argc, char ** argv)
{
	modulight::Application app(argc, argv);

	ParallelProcess * parallelProcess = app.addParallelProcess("5_parallel_module", 4);
	Process * generator = app.addProcess("5_generator");

	// All the processes within the parallel process will receive the same data
	app.connect(generator, "out", parallelProcess, "in");

	app.start();
	return 0;
}