#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <QString>
#include <QVector>

namespace modulight
{

struct NetworkModuleDescription
{
    QString name;
    int instance;
    QVector<QString> inputPorts;
    QVector<QString> outputPorts;
};

struct NetworkConnection
{
    QString sourceName;
    int sourceInstance;
    QString sourcePort;

    QString destinationName;
    int destinationInstance;
    QString destinationPort;
};

struct Network
{
    QVector<NetworkModuleDescription> modules;
    QVector<NetworkConnection> connections;

    QString toXML(int indent = -1) const;
    QString toDot() const;
    QString toDotSimplified() const;
};

}

#endif // NETWORK_HPP
