#include <modulight/common/arguments.hpp>

#include <modulight/common/xml.hpp>
#include <modulight/common/modulightexception.hpp>

#include <iostream>

#include <QDebug>
#include <QFile>
#include <QRegExp>

using namespace std;

void modulight::Arguments::addString(const QString &arg, const QString &value)
{
    _map[arg] = value;
}

void modulight::Arguments::addInt(const QString &arg, int value)
{
    _map[arg] = QString("%1").arg(value);
}

void modulight::Arguments::addFloat(const QString &arg, float value)
{
    _map[arg] = QString("%1").arg(value);
}

void modulight::Arguments::addDouble(const QString &arg, double value)
{
    _map[arg] = QString("%1").arg(value);
}

void modulight::Arguments::addBool(const QString &arg, bool value)
{
    _map[arg] = (value ? "true" : "false");
}

QString modulight::Arguments::getString(const QString &arg, const QString &defaultValue) const
{
    if (_map.contains(arg))
        return _map[arg];

    return defaultValue;
}

int modulight::Arguments::getInt(const QString &arg, int defaultValue) const
{
    if (_map.contains(arg))
    {
        bool ok;
        int res = _map[arg].toInt(&ok);

        if (ok)
            return res;
        else
            cerr << "Warning : argument \"" << arg.toStdString() << "\" is not an int" << endl;
    }

    return defaultValue;
}

float modulight::Arguments::getFloat(const QString &arg, float defaultValue) const
{
    if (_map.contains(arg))
    {
        bool ok;
        float res = _map[arg].toFloat(&ok);

        if (ok)
            return res;
        else
            cerr << "Warning : argument \"" << arg.toStdString() << "\" is not a float" << endl;
    }

    return defaultValue;
}

double modulight::Arguments::getDouble(const QString &arg, double defaultValue) const
{
    if (_map.contains(arg))
    {
        bool ok;
        float res = _map[arg].toDouble(&ok);

        if (ok)
            return res;
        else
            cerr << "Warning : argument \"" << arg.toStdString() << "\" is not a double" << endl;
    }

    return defaultValue;
}

bool modulight::Arguments::getBool(const QString &arg, bool defaultValue) const
{
    if (_map.contains(arg))
    {
        QString value = _map[arg];

        if (value == "true")
            return true;
        else if (value == "false")
            return false;
        else
            cerr << "Warning : argument \"" << arg.toStdString() << "\" is not a bool (" << value.toStdString() << ')' << endl;
    }

    return defaultValue;
}

bool modulight::Arguments::isEmpty() const
{
    return _map.isEmpty();
}

QString modulight::Arguments::toXML() const
{
    QString xmlString;
    xml::writeArguments(xmlString, _map);

    return xmlString;
}

void modulight::Arguments::loadFromXML(const QString &xmlString)
{
    xml::readArguments(xmlString, _map);
}
