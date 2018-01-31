#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <QString>
#include <QVector>

#include <mpi.h>

#include <modulight/common/moduledescription.hpp>
#include <modulight/common/arguments.hpp>

namespace modulight
{
struct Process
{
    int id;
    QString command;
    ArgumentWriter args;

    MPI_Comm comm;
    int rank;

    bool canAlterNetwork;
    bool isFinished;
    bool isAboutToBeFinished;

    ModuleDescription description;
    int instanceNumber;

    Process();

    bool operator==(const Process & p) const;
    bool operator<(const Process & p) const;
};
}

#endif // PROCESS_HPP
