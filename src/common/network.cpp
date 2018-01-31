#include <modulight/common/network.hpp>

#include <modulight/common/xml.hpp>
#include <modulight/common/dot.hpp>

QString modulight::Network::toXML(int indent) const
{
    QString s;
    xml::writeNetwork(s, *this, indent);

    return s;
}

QString modulight::Network::toDot() const
{
    QString s;
    dot::networkToDot(*this, s);

    return s;
}

QString modulight::Network::toDotSimplified() const
{
    QString s;
    dot::networkToDotSimplified(*this, s);

    return s;
}
