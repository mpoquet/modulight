#include <modulight/common/dot.hpp>

#include <QStringList>

void modulight::dot::networkToDot(const Network & network, QString & dot)
{
    dot ="digraph network {\n";

    QVector<NetworkModuleDescription>::const_iterator itmod = network.modules.constBegin();
    for(; itmod != network.modules.constEnd(); itmod++)
    {
        networkModuleDescriptionToDot(*itmod,dot);
        dot +="\n";
    }

    QVector<NetworkConnection>::const_iterator itcon = network.connections.constBegin();
    for(; itcon != network.connections.constEnd(); itcon++)
    {
        QString src, dest;
        src = "\"" + itcon->sourceName + QString("%1").arg(itcon->sourceInstance) + "\":\"" + itcon->sourcePort + "\"";
        dest = "\"" + itcon->destinationName + QString("%1").arg(itcon->destinationInstance) + "\":\"" + itcon->destinationPort + "\"";
        dot += src + " -> " + dest +";\n";
    }

    dot += "}";
}

void modulight::dot::networkToDotSimplified(const Network & network, QString & dot)
{
    dot ="digraph network {\nnode [shape=box,style=rounded]\n";

    QVector<NetworkModuleDescription>::const_iterator itmod = network.modules.constBegin();
    for(; itmod != network.modules.constEnd(); itmod++)
    {
        dot += QString("\"%1%2\"").arg(itmod->name).arg(itmod->instance) + " [label=\"" + itmod->name + "(" + QString("%1").arg(itmod->instance) + ")\"];";
        dot +="\n";
    }
    QMap<QString, QString> map;
    QVector<NetworkConnection>::const_iterator itcon = network.connections.constBegin();
    for(; itcon != network.connections.constEnd(); itcon++)
    {
        QString src, dest;
        src = "\"" + itcon->sourceName + QString("%1").arg(itcon->sourceInstance) + "\"";
        dest = "\"" + itcon->destinationName + QString("%1").arg(itcon->destinationInstance) + "\"";
        if(map.contains(src))
        {
            QStringList list = map[src].split(",");
            if(!list.contains(dest))
            {
                dot += src + " -> " + dest +";\n";
                map[src] += "," + dest;
            }
        }
        else
        {
            dot += src + " -> " + dest +";\n";
            map[src] = dest;
        }
    }

    dot += "}";
}

void modulight::dot::moduleDescriptionToDot(const ModuleDescription & description, QString & dot)
{
    dot = "\"" + description.name + "\" [label=\"{";

    if(!description.inputPorts.isEmpty())
    {
        dot +="{";

        QVectorIterator<QString> itiport(description.inputPorts);
        while (itiport.hasNext())
        {
            QString s = itiport.next();

            dot += "<" + s + "> " + s;

            if (itiport.hasNext())
                dot += "|=";
        }

        dot += "}|";
    }

    dot += description.name;

    if(!description.outputPorts.isEmpty())
    {
        dot += "|{";

        QMapIterator<QString, ModuleDescription::OutputPort> it(description.outputPorts);
        while (it.hasNext())
        {
            it.next();

            dot += "<" + it.key() + "> " + it.key();

            if (it.hasNext())
                dot += "|=";
        }

        dot += "}";
    }

    dot += "}\",shape=Mrecord];";
}

void modulight::dot::networkModuleDescriptionToDot(const NetworkModuleDescription & description, QString & dot)
{
    dot += "\"" + description.name + QString("%1").arg(description.instance) + "\" [label=\"{";
    if(description.inputPorts.size() > 0)
    {
        dot +="{";

        QVector<QString>::const_iterator itiport = description.inputPorts.constBegin();
        for(; itiport != description.inputPorts.constEnd(); itiport++)
        {
            dot += "<" + *itiport+"> "+*itiport;
            if((itiport)+1 != description.inputPorts.constEnd())
                dot +="|";
        }
        dot +="}|";
    }
    dot += description.name + "(" + QString("%1").arg(description.instance) + ")";
    if(description.outputPorts.size()>0)
    {
        dot +="|{";

        QVector<QString>::const_iterator itiport = description.outputPorts.constBegin();
        for(; itiport != description.outputPorts.constEnd(); itiport++)
        {
            dot += "<" + *itiport+"> "+*itiport;
            if((itiport)+1 != description.outputPorts.constEnd())
                dot +="|";
        }

        dot +="}";
    }
    dot += "}\",shape=Mrecord];";
}
