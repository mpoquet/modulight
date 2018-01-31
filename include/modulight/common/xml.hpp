#ifndef XML_HPP
#define XML_HPP

#include <QString>

#include <modulight/common/moduledescription.hpp>
#include <modulight/common/sequence.hpp>
#include <modulight/common/network.hpp>
#include <modulight/common/dynamicrequest.hpp>
#include <modulight/common/dynamicorder.hpp>

namespace modulight
{
/**
 * @brief Contains functions to convert to and from XML
 */
namespace xml
{
/**
     * @brief readArguments Allows to fill a map of arguments from a XML string
     * @param xml The XML string
     * @param arguments The argument map
     */
    void readArguments(const QString & xml, QMap<QString, QString> & arguments);

    /**
     * @brief writeArguments Allows to write a XML string from a map of arguments
     * @param xml The XML string
     * @param arguments The argument map
     * @param indent The output indentation (-1 to compress, 4 to be readable)
     */
    void writeArguments(QString & xml, const QMap<QString, QString> & arguments, int indent = -1);

    /**
     * @brief Allows to get a module description from a XML string
     * @param xml The XML string
     * @param description The module description
     */
    void readModuleDescription(const QString & xml, ModuleDescription & description);

    /**
     * @brief Allows to get a XML string from a module description
     * @param xml The XML string
     * @param description The module description
     * @param indent The output indentation (-1 to compress, 4 to be readable)
     */
    void writeModuleDescription(QString & xml, const ModuleDescription & description, int indent = -1);

    /**
     * @brief readSequence Allows to get a Sequence from a XML string
     * @param xml The XML string
     * @param sequence The Sequence
     */
    void readSequence(const QString & xml, Sequence & sequence);

    /**
     * @brief readSequence Allows to get a XML string from a Sequence
     * @param xml The XML string
     * @param sequence The Sequence
     * @param indent The output indentation (-1 to compress, 4 to be readable)
     */
    void writeSequence(QString & xml, const Sequence & sequence, int indent = -1);

    /**
     * @brief readNetwork Allows to get a Network from a XML string
     * @param xml The XML string
     * @param network The Network
     */
    void readNetwork(const QString & xml, Network & network);

    /**
     * @brief writeNetwork Allows to get a XML string from a Network
     * @param xml The XML string
     * @param network The Network
     * @param indent The output indentation (-1 to compress, 4 to be readable)
     */
    void writeNetwork(QString & xml, const Network & network, int indent = -1);

    /**
     * @brief readDynamicRequests Allows to get a DynamicRequestSequence from a XML string
     * @param xml The XML string
     * @param requests The DynamicRequestSequence
     */
    void readDynamicRequests(const QString & xml, DynamicRequestSequence & requests);

    /**
     * @brief writeDynamicRequests Allows to get a XML string from a DynamicRequestSequence
     * @param xml The XML string
     * @param requests The DynamicRequestSequence
     * @param indent The output indentation (-1 to compress, 4 to be readable)
     */
    void writeDynamicRequests(QString & xml, const DynamicRequestSequence & requests, int indent = -1);

    /**
     * @brief readDynamicOrders Allows to get a DynamicOrderSequence from a XML string
     * @param xml The XML string
     * @param orders The DynamicOrderSequence
     */
    void readDynamicOrders(const QString & xml, DynamicOrderSequence & orders);

    /**
     * @brief writeDynamicOrders Allows to get a XML string from a DynamicOrderSequence
     * @param xml The XML string
     * @param orders The dynamicOrderSequence@param indent The output indentation (-1 to compress, 4 to be readable)
     * @param indent The output indentation (-1 to compress, 4 to be readable)
     */
    void writeDynamicOrders(QString & xml, const DynamicOrderSequence & orders, int indent = -1);
}
}

#endif // XML_HPP
