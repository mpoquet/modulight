#ifndef PORT_HPP
#define PORT_HPP

#include <QVector>
#include <QString>
#include <QRegExp>
#include <QStringList>

#include <zmq.hpp>

#include <modulight/common/modulightexception.hpp>
#include <modulight/module/stamp.hpp>

namespace modulight
{

struct InputPort
{
    zmq::socket_t * sub;
    zmq::socket_t * req;

    QByteArray completePortName; // For example, Bouh2:in

    bool messageAvailableOnLossless;
    bool messageAvailableOnLossy;

    QStringList losslessRemotes;
    QStringList lossyRemotes;

    InputPort() : sub(0), req(0) {}
};

struct OutputPort
{
    zmq::socket_t * pub;
    zmq::socket_t * rep;

    quint16 pubPort;
    quint16 repPort;

    QByteArray completePortName; // For example, Module42:out

    QVector<char> lastMessageBuffer;
    Stamp lastStamp;

    QStringList losslessRemotes;
    QMap<QString, bool> lossyRemotes; // true => the current message had been sent to the remote

    int iterationNumber;

    OutputPort() : pub(0), rep(0), iterationNumber(-1) {}
};

}

#endif // PORT_HPP
