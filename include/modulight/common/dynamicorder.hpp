#ifndef DYNAMICORDER_HPP
#define DYNAMICORDER_HPP

#include <QString>

namespace modulight
{
/**
 * @brief Contains types of DynamicOrder
 */
namespace OrderType
{
    /**
     * @brief Contains types of DynamicOrder
     */
    enum OrderType
    {
        ACCEPT,
        CONNECT,
        INPUT_DISCONNECT,
        OUTPUT_DISCONNECT,
        DESTROY
    };
}

struct DynamicOrder
{
    OrderType::OrderType type;

    // These are used when connections are being altered (ACCEPT, CONNECT, INPUT_DISCONNECT, OUTPUT_DISCONNECT)
    bool lossyConnection;
    QString localPortName;
    QString remoteAbbrevName;
    QString remoteIP;
    quint16 remotePort;
    quint16 syncPort;
};

struct DynamicOrderSequence
{
    QVector<DynamicOrder> orders;
};
}

#endif // DYNAMICORDER_HPP
