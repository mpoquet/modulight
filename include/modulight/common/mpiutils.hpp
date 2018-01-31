#ifndef MPIUTILS_HPP
#define MPIUTILS_HPP

#include <mpi.h>

#include <QString>

namespace modulight
{
/**
 * @brief Internal easy-to-use functions to send and receive strings via MPI
 */
namespace mpi_util
{
    void sendQString(const QString & s, int dest, int tag, MPI_Comm comm);
    void ssendQString(const QString & s, int dest, int tag, MPI_Comm comm);

    void recvQString(QString & s, int source, int tag, MPI_Comm comm, MPI_Status * status);
    void recvQStringNoProbe(QString & s, int size, int source, int tag, MPI_Comm comm, MPI_Status * status);
}
}

#endif // MPIUTILS_HPP
