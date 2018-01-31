#ifndef PENDINGCONNECTION_HPP
#define PENDINGCONNECTION_HPP

#include <QString>

#include <modulight/master/process.hpp>

namespace modulight
{
struct MasterConnection
{
    int processA;
    QString portA;

    int processB;
    QString portB;

    bool lossy;

    bool operator==(const MasterConnection & c);
};
}

#endif // PENDINGCONNECTION_HPP
