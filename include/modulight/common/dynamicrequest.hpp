#ifndef DYNAMICREQUEST_HPP
#define DYNAMICREQUEST_HPP

#include <QString>
#include <QMap>

namespace modulight
{
/**
 * @brief Contains types of DynamicRequest
 */
namespace RequestType
{
    /**
     * @brief Contains types of DynamicRequest
     */
    enum RequestType
    {
        ADD_MODULE,
        ADD_CONNECTION,
        REMOVE_CONNECTION,
        REMOVE_MODULE
    };
}

struct DynamicRequest
{
    RequestType::RequestType type;  // *

    QString path;                   // ADD_MODULE
    QMap<QString, QString> args;    // ADD_MODULE
    QString host;                   // ADD_MODULE

    QString sourceName;             // ADD_CONNECTION, REMOVE_CONNECTION
    int sourceInstance;             // ADD_CONNECTION, REMOVE_CONNECTION
    QString sourcePort;             // ADD_CONNECTION, REMOVE_CONNECTION

    QString destinationName;        // ADD_CONNECTION, REMOVE_CONNECTION
    int destinationInstance;        // ADD_CONNECTION, REMOVE_CONNECTION
    QString destinationPort;        // ADD_CONNECTION, REMOVE_CONNECTION

    bool lossyConnection;           // ADD_CONNECTION

    QString moduleName;             // REMOVE_MODULE
    int moduleInstance;             // REMOVE_MODULE
};

struct DynamicRequestSequence
{
    QVector<DynamicRequest> requests;
};
}

#endif // DYNAMICREQUEST_HPP
