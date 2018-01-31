#include <modulight/common/xml.hpp>

#include <iostream>

#include <QDomDocument>
#include <QString>
#include <QDebug>

using namespace std;
using namespace modulight::RequestType;
using namespace modulight::OrderType;

void modulight::xml::readArguments(const QString & xml, QMap<QString, QString> & arguments)
{
    QDomDocument doc;
    doc.setContent(xml);

    QDomNode node = doc.documentElement().firstChild();
    for (; !node.isNull(); node = node.nextSibling())
    {
        if (!node.isComment())
        {
            QDomElement element = node.toElement();
            arguments[element.attributeNode("name").value()] = element.attributeNode("value").value();
        }
    }
}

void modulight::xml::writeArguments(QString & xml, const QMap<QString, QString> & arguments, int indent)
{
    QDomDocument doc;

    QDomElement docElem = doc.createElement("args");
    doc.appendChild(docElem);

    QMap<QString, QString>::ConstIterator it = arguments.constBegin();
    for(; it != arguments.constEnd(); it++)
    {
        QDomElement elem = doc.createElement("arg");
        elem.setAttribute("name",it.key());
        elem.setAttribute("value",it.value());
        docElem.appendChild(elem);
    }

    xml = doc.toString(indent);
}

void modulight::xml::readModuleDescription(const QString & xml, ModuleDescription & description)
{
    description.inputPorts.clear();
    description.outputPorts.clear();

    QDomDocument doc;
    doc.setContent(xml);

    QDomElement docElem = doc.documentElement();

    description.name = docElem.attribute("name");
    description.ip = docElem.attribute("ip");
    description.syncPort = docElem.attribute("syncPort").toInt();

    QDomElement child = docElem.firstChild().toElement();
    for(; !child.isNull(); child = child.nextSibling().toElement())
    {
        if(child.nodeName() == "iport")
        {
            description.inputPorts.append(child.attribute("name"));
        }
        else if(child.nodeName() == "oport")
        {
            ModuleDescription::OutputPort o;

            o.losslessPort = child.attribute("losslessPort").toInt();
            o.lossyPort = child.attribute("lossyPort").toInt();

            description.outputPorts[child.attribute("name")] = o;
        }
    }
}

void modulight::xml::writeModuleDescription(QString & xml, const ModuleDescription & description, int indent)
{
    QDomDocument doc;

    QDomElement docelem = doc.createElement("module");
    doc.appendChild(docelem);

    docelem.setAttribute("name", description.name);
    docelem.setAttribute("ip", description.ip);
    docelem.setAttribute("syncPort", description.syncPort);

    foreach(QString inputPort, description.inputPorts)
    {
        QDomElement iport = doc.createElement("iport");
        iport.setAttribute("name", inputPort);
        docelem.appendChild(iport);
    }

    QMapIterator<QString, ModuleDescription::OutputPort> it(description.outputPorts);
    while (it.hasNext())
    {
        it.next();

        QDomElement oport = doc.createElement("oport");
        oport.setAttribute("name", it.key());
        oport.setAttribute("losslessPort", it.value().losslessPort);
        oport.setAttribute("lossyPort", it.value().lossyPort);

        docelem.appendChild(oport);
    }

    xml = doc.toString(indent);
}

void modulight::xml::readSequence(const QString & xml, Sequence & sequence)
{
    sequence.connections.clear();

    QDomDocument doc;
    doc.setContent(xml);

    QDomElement docElem = doc.documentElement();

    QDomElement child = docElem.firstChild().toElement();
    for(; !child.isNull(); child = child.nextSibling().toElement())
    {
        if(child.nodeName() == "connect")
        {
            Connection c;
            c.isConnect = true;
            c.isLossy = child.attribute("lossy").toInt();
            c.localPortName = child.attribute("localPortName");
            c.remoteIP = child.attribute("remoteIP");
            c.remotePort = child.attribute("remotePort").toInt();
            c.syncPort = child.attribute("syncPort").toInt();
            c.remoteAbbrevName = child.attribute("remoteAbbrevName");

            sequence.connections.append(c);
        }
        else if (child.nodeName() == "accept")
        {
            Connection c;
            c.isConnect = false;
            c.isLossy = child.attribute("lossy").toInt();
            c.localPortName = child.attribute("localPortName");
            c.remoteAbbrevName = child.attribute("remoteAbbrevName");

            sequence.connections.append(c);
        }
    }
}

void modulight::xml::writeSequence(QString & xml, const Sequence & sequence, int indent)
{
    QDomDocument doc("");
    QDomElement docElem = doc.createElement("sequence");
    doc.appendChild(docElem);

    foreach(Connection c, sequence.connections)
    {
        if (c.isConnect)
        {
            QDomElement node = doc.createElement("connect");
            node.setAttribute("lossy", c.isLossy);
            node.setAttribute("localPortName", c.localPortName);
            node.setAttribute("remoteIP", c.remoteIP);
            node.setAttribute("remotePort", c.remotePort);
            node.setAttribute("syncPort", c.syncPort);
            node.setAttribute("remoteAbbrevName", c.remoteAbbrevName);

            docElem.appendChild(node);
        }
        else
        {
            QDomElement node = doc.createElement("accept");
            node.setAttribute("lossy", c.isLossy);
            node.setAttribute("localPortName", c.localPortName);
            node.setAttribute("remoteAbbrevName", c.remoteAbbrevName);

            docElem.appendChild(node);
        }
    }

    xml = doc.toString(indent);
}

void modulight::xml::readNetwork(const QString &xml, Network &network)
{
    network.connections.clear();
    network.modules.clear();

    QDomDocument doc;
    doc.setContent(xml);

    QDomElement docelem = doc.documentElement();
    QDomElement child = docelem.firstChild().toElement();
    for(; !child.isNull(); child = child.nextSibling().toElement())
    {
        if(child.nodeName() == "module")
        {
            NetworkModuleDescription netmod;
            netmod.name = child.attribute("name");
            netmod.instance = child.attribute("instance").toInt();

            QDomElement modchild = child.firstChild().toElement();
            for(; !modchild.isNull(); modchild = modchild.nextSibling().toElement())
            {
                if(modchild.nodeName() == "iport")
                    netmod.inputPorts.append(modchild.attribute("name"));
                else if(modchild.nodeName() == "oport")
                    netmod.outputPorts.append(modchild.attribute("name"));
            }

            network.modules.append(netmod);
        }
        else if(child.nodeName() == "connection")
        {
            NetworkConnection netcon;

            QDomElement conchild = child.firstChild().toElement();
            for(; !conchild.isNull(); conchild = conchild.nextSibling().toElement())
            {

                if(conchild.nodeName() == "source")
                {
                    netcon.sourceName = conchild.attribute("name");
                    netcon.sourceInstance = conchild.attribute("instance").toInt();
                    netcon.sourcePort = conchild.attribute("port");
                }
                else if(conchild.nodeName() == "destination")
                {
                    netcon.destinationName = conchild.attribute("name");
                    netcon.destinationInstance = conchild.attribute("instance").toInt();
                    netcon.destinationPort = conchild.attribute("port");
                }

            }

            network.connections.append(netcon);
        }
    }
}

void modulight::xml::writeNetwork(QString &xml, const Network &network, int indent)
{
    QDomDocument doc;

    QDomElement docelem = doc.createElement("network");
    doc.appendChild(docelem);

    QVector<NetworkModuleDescription>::const_iterator it = network.modules.constBegin();
    for(; it != network.modules.constEnd(); it++)
    {
        QDomElement mod = doc.createElement("module");
        mod.setAttribute("name",it->name);
        mod.setAttribute("instance",it->instance);

        QVector<QString>::const_iterator itiport = it->inputPorts.constBegin();
        for(; itiport != it->inputPorts.constEnd(); itiport++)
        {
            QDomElement iport = doc.createElement("iport");
            iport.setAttribute("name",*itiport);
            mod.appendChild(iport);
        }

        QVector<QString>::const_iterator itoport = it->outputPorts.constBegin();
        for(; itoport != it->outputPorts.constEnd(); itoport++)
        {
            QDomElement oport = doc.createElement("oport");
            oport.setAttribute("name",*itoport);
            mod.appendChild(oport);
        }

        docelem.appendChild(mod);
    }

    QVector<NetworkConnection>::const_iterator itcon = network.connections.constBegin();
    for(; itcon != network.connections.constEnd(); itcon++)
    {
        QDomElement con = doc.createElement("connection");

        QDomElement src = doc.createElement("source");
        src.setAttribute("name",itcon->sourceName);
        src.setAttribute("instance",itcon->sourceInstance);
        src.setAttribute("port",itcon->sourcePort);
        con.appendChild(src);

        QDomElement dest = doc.createElement("destination");
        dest.setAttribute("name",itcon->destinationName);
        dest.setAttribute("instance",itcon->destinationInstance);
        dest.setAttribute("port",itcon->destinationPort);
        con.appendChild(dest);

        docelem.appendChild(con);
    }

    xml = doc.toString(indent);
}

void modulight::xml::readDynamicRequests(const QString & xml, DynamicRequestSequence & requests)
{
    requests.requests.clear();

    QDomDocument doc;
    doc.setContent(xml);

    QDomElement docelem = doc.documentElement();

    QDomElement child = docelem.firstChild().toElement();
    for(; !child.isNull(); child = child.nextSibling().toElement())
    {
        DynamicRequest req;

        if(child.nodeName() == "add_module")
        {
            req.type = ADD_MODULE;
            req.path = child.attribute("path");
            req.host = child.attribute("host"); // todo after check

            QDomElement args = child.firstChild().toElement();

            QDomElement arg = args.firstChild().toElement();
            for(; !arg.isNull(); arg = arg.nextSibling().toElement())
                req.args[arg.attribute("name")]=arg.attribute("value");

        }
        else if(child.nodeName() == "add_connection")
        {
            req.type = ADD_CONNECTION;
            req.lossyConnection = child.attribute("lossyConnection").toInt();

            QDomElement conchild = child.firstChild().toElement();
            for(; !conchild.isNull(); conchild = conchild.nextSibling().toElement())
            {
                if(conchild.nodeName() == "source")
                {
                    req.sourceName = conchild.attribute("name");
                    req.sourceInstance = conchild.attribute("instance").toInt();
                    req.sourcePort = conchild.attribute("port");
                }
                else if(conchild.nodeName() == "destination")
                {
                    req.destinationName = conchild.attribute("name");
                    req.destinationInstance = conchild.attribute("instance").toInt();
                    req.destinationPort = conchild.attribute("port");
                }
            }
        }
        else if(child.nodeName() == "remove_connection")
        {
            req.type = REMOVE_CONNECTION;

            QDomElement conchild = child.firstChild().toElement();
            for(; !conchild.isNull(); conchild = conchild.nextSibling().toElement())
            {
                if(conchild.nodeName() == "source")
                {
                    req.sourceName = conchild.attribute("name");
                    req.sourceInstance = conchild.attribute("instance").toInt();
                    req.sourcePort = conchild.attribute("port");
                }
                else if(conchild.nodeName() == "destination")
                {
                    req.destinationName = conchild.attribute("name");
                    req.destinationInstance = conchild.attribute("instance").toInt();
                    req.destinationPort = conchild.attribute("port");
                }
            }
        }
        else if(child.nodeName() == "remove_module")
        {
            req.type = REMOVE_MODULE;

            req.moduleName = child.attribute("name");
            req.moduleInstance = child.attribute("instance").toInt();
        }

        requests.requests.append(req);
    }
}

void modulight::xml::writeDynamicRequests(QString & xml, const DynamicRequestSequence & requests, int indent)
{
    QDomDocument doc;

    QDomElement docelem = doc.createElement("dynamic_request");
    doc.appendChild(docelem);

    QVector<DynamicRequest>::const_iterator it = requests.requests.constBegin();
    for(; it != requests.requests.constEnd(); it++)
    {
        QDomElement req = doc.createElement("newelem");

        switch(it->type)
        {
        case ADD_MODULE:
        {
            req.setTagName("add_module");
            req.setAttribute("path",it->path);
            req.setAttribute("host",it->host);

            QDomElement args = doc.createElement("args");
            req.appendChild(args);

            QMap<QString, QString>::ConstIterator itarg = it->args.constBegin();
            for(; itarg != it->args.constEnd(); itarg++)
            {
                QDomElement arg = doc.createElement("arg");
                arg.setAttribute("name", itarg.key());
                arg.setAttribute("value", itarg.value());
                args.appendChild(arg);
            }

            break;
        }
        case ADD_CONNECTION:
        {
            req.setTagName("add_connection");
            req.setAttribute("lossyConnection", it->lossyConnection);

            QDomElement src = doc.createElement("source");
            src.setAttribute("name",it->sourceName);
            src.setAttribute("instance",it->sourceInstance);
            src.setAttribute("port",it->sourcePort);
            req.appendChild(src);

            QDomElement dest = doc.createElement("destination");
            dest.setAttribute("name",it->destinationName);
            dest.setAttribute("instance",it->destinationInstance);
            dest.setAttribute("port",it->destinationPort);
            req.appendChild(dest);

            break;
        }
        case REMOVE_CONNECTION:
        {
            req.setTagName("remove_connection");
            QDomElement src = doc.createElement("source");
            src.setAttribute("name",it->sourceName);
            src.setAttribute("instance",it->sourceInstance);
            src.setAttribute("port",it->sourcePort);
            req.appendChild(src);
            QDomElement dest = doc.createElement("destination");
            dest.setAttribute("name",it->destinationName);
            dest.setAttribute("instance",it->destinationInstance);
            dest.setAttribute("port",it->destinationPort);
            req.appendChild(dest);
            break;
        }
        case REMOVE_MODULE:
        {
            req.setTagName("remove_module");
            req.setAttribute("name",it->moduleName);
            req.setAttribute("instance",it->moduleInstance);
            break;
        }
        }

        docelem.appendChild(req);
    }

    xml = doc.toString(indent);
}

void modulight::xml::readDynamicOrders(const QString & xml, DynamicOrderSequence & orders)
{
    orders.orders.clear();

    QDomDocument doc;
    doc.setContent(xml);

    QDomElement docelem = doc.documentElement();

    QDomElement child = docelem.firstChild().toElement();
    for(; !child.isNull(); child = child.nextSibling().toElement())
    {
        DynamicOrder order;
        if(child.nodeName() == "accept")
        {
            order.type = ACCEPT;
            order.localPortName = child.attribute("localPortName");
            order.remoteAbbrevName = child.attribute("remoteAbbrevName");
            order.lossyConnection = child.attribute("lossyConnection").toInt();
        }
        else if(child.nodeName() == "connect")
        {
            order.type = CONNECT;
            order.localPortName = child.attribute("localPortName");
            order.remoteAbbrevName = child.attribute("remoteAbbrevName");
            order.remoteIP = child.attribute("remoteIP");
            order.remotePort = child.attribute("remotePort").toInt();
            order.syncPort = child.attribute("syncPort").toInt();
            order.lossyConnection = child.attribute("lossyConnection").toInt();
        }
        else if(child.nodeName() == "idisconnect")
        {
            order.type = INPUT_DISCONNECT;
            order.localPortName = child.attribute("localPortName");
            order.remoteAbbrevName = child.attribute("remoteAbbrevName");
            order.remoteIP = child.attribute("remoteIP");
            order.remotePort = child.attribute("remotePort").toInt();
            order.syncPort = child.attribute("syncPort").toInt();
            order.lossyConnection = child.attribute("lossyConnection").toInt();
        }
        else if(child.nodeName() == "odisconnect")
        {
            order.type = OUTPUT_DISCONNECT;
            order.localPortName = child.attribute("localPortName");
            order.remoteAbbrevName = child.attribute("remoteAbbrevName");
            order.lossyConnection = child.attribute("lossyConnection").toInt();
        }
        else if(child.nodeName() == "destroy")
            order.type = DESTROY;
        orders.orders.append(order);
    }
}

void modulight::xml::writeDynamicOrders(QString & xml, const DynamicOrderSequence & orders, int indent)
{
    QDomDocument doc;

    QDomElement docelem = doc.createElement("dynamic_order");
    doc.appendChild(docelem);

    QVectorIterator<DynamicOrder> it(orders.orders);
    while (it.hasNext())
    {
        DynamicOrder o = it.next();

        QDomElement ord = doc.createElement("order");
        switch(o.type)
        {
        case ACCEPT:
            ord.setTagName("accept");
            ord.setAttribute("localPortName", o.localPortName);
            ord.setAttribute("remoteAbbrevName", o.remoteAbbrevName);
            ord.setAttribute("lossyConnection", o.lossyConnection);
            break;
        case CONNECT:
            ord.setTagName("connect");
            ord.setAttribute("localPortName", o.localPortName);
            ord.setAttribute("remoteAbbrevName", o.remoteAbbrevName);
            ord.setAttribute("remoteIP", o.remoteIP);
            ord.setAttribute("remotePort", o.remotePort);
            ord.setAttribute("syncPort", o.syncPort);
            ord.setAttribute("lossyConnection", o.lossyConnection);
            break;
        case INPUT_DISCONNECT:
            ord.setTagName("idisconnect");
            ord.setAttribute("localPortName", o.localPortName);
            ord.setAttribute("remoteAbbrevName", o.remoteAbbrevName);
            ord.setAttribute("remoteIP", o.remoteIP);
            ord.setAttribute("remotePort", o.remotePort);
            ord.setAttribute("syncPort", o.syncPort);
            ord.setAttribute("lossyConnection", o.lossyConnection);
            break;
        case OUTPUT_DISCONNECT:
            ord.setTagName("odisconnect");
            ord.setAttribute("localPortName", o.localPortName);
            ord.setAttribute("remoteAbbrevName", o.remoteAbbrevName);
            ord.setAttribute("lossyConnection", o.lossyConnection);
            break;
        case DESTROY:
            ord.setTagName("destroy");
        }
        docelem.appendChild(ord);
    }

    xml = doc.toString(indent);
}
