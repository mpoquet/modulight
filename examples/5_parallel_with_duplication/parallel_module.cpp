#include <iostream>
#include <unistd.h>
#include <modulight/module.hpp>
#include <QTime>

using namespace std;
using namespace modulight;

int main(int argc, char ** argv)
{
	Module m("ParallelModule", argc, argv);
	m.addInputPort("in");

	if (!m.initialize())
		return 0;
	
	MPI_Comm comm = m.mpiWorld();
	int rank, size;

	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &size);

	m.display() << "(rank,size) = (" << rank << ',' << size << ')' << endl;

	QTime time;
	time.start();

	while (m.wait("in", 1000))
	{
		MessageReader reader;

		if (m.readMessage("in", reader))
		{
			float * data = (float*) reader.data();
			int dataSize = reader.size() / sizeof(float);

			// Parallel computation... This example finds the minimum of the given array

			int begin = rank * (dataSize / size);
			int end = (rank+1) * (dataSize / size) - 1;

			if (rank == size - 1)
				end = dataSize - 1;
			
			int min = begin;
			for (int i = begin+1; i < end; ++i)
				if (data[i] < data[min])
					min = i;

			// Sharing results
			float minValue = data[min];
			float overallMin;
			MPI_Allreduce(&minValue, &overallMin, 1, MPI_FLOAT, MPI_MIN, comm);

			if (rank == 0)
				m.display() << "Overall minimum is " << overallMin << endl;
		}
	}

	m.display() << "no data received in 1000 ms, aborting" << endl;

	MPI_Barrier(comm);

	return 0;
}