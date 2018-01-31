#ifndef MODULEDESCRIPTION_HPP
#define MODULEDESCRIPTION_HPP

#include <QString>
#include <QMap>
#include <QVector>

namespace modulight
{
struct ModuleDescription
{
    struct OutputPort
    {
        quint16 losslessPort; // tcp port number
        quint16 lossyPort; // tcp port number
    };

    QString name;
    QString ip;
    quint16 syncPort;
    QVector<QString> inputPorts;
    QMap<QString, OutputPort> outputPorts;
};
}

#endif // MODULEDESCRIPTION_HPP
