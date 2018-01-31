#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP

#include <QString>
#include <QVector>

namespace modulight
{
struct Connection
{
    // The inputport (sub) will subscribe to the outputport (pub)
    bool isConnect; // true -> connect, false -> useless arguments except localPortName and remoteAbbrevName
    bool isLossy;

    QString localPortName; // Modulight port, not a TCP one
    QString remoteAbbrevName; // For example, A0:out
    QString remoteIP;
    quint16 remotePort; // TCP port number
    quint16 syncPort; // TCP port number
};

struct Sequence
{
    QVector<Connection> connections;
};
}

#endif // SEQUENCE_HPP
