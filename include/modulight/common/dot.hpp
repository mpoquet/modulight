#ifndef DOT_HPP
#define DOT_HPP

#include <QString>

#include <modulight/common/network.hpp>
#include <modulight/common/moduledescription.hpp>

namespace modulight
{
/**
 * @brief Contains functions to convert to and from DOT (graphviz file format)
 */
namespace dot
{
    /**
     * @brief networkToDot Allows to get a dot string from a Network
     * @param network The Network
     * @param dot The dot string
     */
    void networkToDot(const Network & network, QString & dot);

    /**
     * @brief networkToDotSimplified Allows to get a simplified dot string from a Network
     * @param network The Network
     * @param dot The dot string
     */
    void networkToDotSimplified(const Network & network, QString & dot);

    /**
     * @brief moduleDescriptionToDot Allows to get a dot string from a ModuleDescription
     * @param description The moduleDescription
     * @param dot The dot string
     */
    void moduleDescriptionToDot(const ModuleDescription & description, QString & dot);

    /**
     * @brief networkModuleDescriptionToDot Allows to get a dot string from a NetworkModuleDescription
     * @param description The networkModuleDescription
     * @param dot the dot string
     */
    void networkModuleDescriptionToDot(const NetworkModuleDescription & description, QString & dot);
}
}

#endif // DOT_HPP
