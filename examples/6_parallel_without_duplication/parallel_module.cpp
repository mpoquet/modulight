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

	if (rank == 0)
	{
		while (m.wait("in", 1000))
		{
			MessageReader reader;

			if (m.readMessage("in", reader))
			{
				float * data = (float*) reader.data();
				int dataSize = reader.size() / sizeof(float);

				// Let's split the array and scatter it
				for (int i = 1; i < size; ++i)
				{
					int begin = i * (dataSize / size);
					int end = (i + 1) * (dataSize / size) - 1;

					if (i == size - 1)
						end = dataSize - 1;

					MPI_Bsend(data + begin, end - begin + 1, MPI_FLOAT, i, 0, comm);
				}

				int myEnd = (dataSize / size) - 1;
				int iMin = 0;
				for (int i = 1; i < myEnd; ++i)
					if (data[i] < data[iMin])
						iMin = i;

				float minValue = data[iMin];
				float overallMin;

				MPI_Reduce(&minValue, &overallMin, 1, MPI_FLOAT, MPI_MIN, 0, comm);
				
				m.display() << "The overall minimum is " << overallMin << endl;
			}
		}

		float u = 42.42;
		for (int i = 1; i < size; ++i)
			MPI_Bsend(&u, 1, MPI_FLOAT, i, 1, comm);

		m.display() << "no message received in 1000 ms, aborting" << endl;
		MPI_Barrier(comm);
	}
	else
	{
		QVector<float> dataVector;

		while (true)
		{
			MPI_Status status;
			MPI_Probe(0, MPI_ANY_TAG, comm, &status);

			if (status.MPI_TAG == 0)
			{
				int myDataSize;
				MPI_Get_count(&status, MPI_FLOAT, &myDataSize);
				dataVector.resize(myDataSize);

				MPI_Recv(dataVector.data(), myDataSize, MPI_FLOAT, 0, 0, comm, MPI_STATUS_IGNORE);

				int iMin = 0;
				for (int i = 1; i < myDataSize; ++i)
					if (dataVector[i] < dataVector[iMin])
						iMin = i;

				float minValue = dataVector[iMin];
				MPI_Reduce(&minValue, 0, 1, MPI_FLOAT, MPI_MIN, 0, comm);
			}
			else
				break;
		}

		MPI_Barrier(comm);
	}

	return 0;
}