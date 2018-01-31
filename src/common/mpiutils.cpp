#include <modulight/common/mpiutils.hpp>

#include <QByteArray>

void modulight::mpi_util::sendQString(const QString & s, int dest, int tag, MPI_Comm comm)
{
    QByteArray qba = s.toUtf8();
    MPI_Send(qba.data(), qba.size(), MPI_CHAR, dest, tag, comm);
}

void modulight::mpi_util::ssendQString(const QString & s, int dest, int tag, MPI_Comm comm)
{
    QByteArray qba = s.toUtf8();
    MPI_Ssend(qba.data(), qba.size(), MPI_CHAR, dest, tag, comm);
}

void modulight::mpi_util::recvQString(QString & s, int source, int tag, MPI_Comm comm, MPI_Status * status)
{
    QByteArray qba;
    MPI_Status probeStatus;
    int size;

    MPI_Probe(source, tag, comm, &probeStatus);
    MPI_Get_count(&probeStatus, MPI_CHAR, &size);

    qba.resize(size);

    MPI_Recv(qba.data(), size, MPI_CHAR, source, tag, comm, status);

    s = QString::fromUtf8(qba);
}

void modulight::mpi_util::recvQStringNoProbe(QString & s, int size, int source, int tag, MPI_Comm comm, MPI_Status * status)
{
    QByteArray qba;
    qba.resize(size);

    MPI_Recv(qba.data(), size, MPI_CHAR, source, tag, comm, status);

    s = QString::fromUtf8(qba);
}
