#include <modulight/master/hostfile.hpp>

#include <stdexcept>
#include <limits>

#include <QFile>
#include <QStringList>
#include <QDebug>
#include <QRegExp>

#include <modulight/common/modulightexception.hpp>

modulight::HostFile::HostFile()
{
    _default = "127.0.0.1";
}

modulight::HostFile::HostFile(const QString &filename)
{
    loadFilename(filename);
}

QString modulight::HostFile::readFile(const QString &filename)
{
    QFile f(filename);

    if (f.open(QIODevice::ReadOnly))
        return f.readAll();
    else
        throw Exception("Impossible to open the specified hostfile");
}

QString modulight::HostFile::uncommented(const QString &string)
{
    QString res;

    QStringList qsl = string.split("\n");

    for (int i = 0; i < qsl.size(); ++i)
    {
        QString str = qsl[i].simplified();

        if (!str.isEmpty())
        {
            if (qsl[i].contains("//"))
            {
                QString s = str.split("//").at(0).simplified();

                if (!s.isEmpty())
                    res += s + '\n';
            }
            else
                res += str + '\n';
        }
    }

    return res;
}

bool modulight::HostFile::isValid()
{
    QMapIterator<QString, QMap<Set, QString> > it(_hosts);

    while (it.hasNext())
    {
        it.next();

        QMapIterator<Set, QString> it2(it.value());

        while (it2.hasNext())
        {
            it2.next();

            QMapIterator<Set, QString> it3(it.value());

            while (it3.hasNext())
            {
                it3.next();

                if (it2.key() != it3.key())
                    if (!it2.key().isDisjointTo(it3.key()))
                        return false;
            }
        }
    }

    return true;
}

QString modulight::HostFile::hostOf(const QString &command, int instance)
{
    if (_hosts.contains(command))
    {
        QMapIterator<Set, QString> it(_hosts[command]);

        while(it.hasNext())
        {
            it.next();

            if (it.key().contains(instance))
                return it.value();
        }

        return _default;
    }
    else
        return _default;
}

void modulight::HostFile::loadFilename(const QString &filename)
{
    QString str = readFile(filename);
    str = uncommented(str);

    QStringList qsl = str.split("\n");

    if (qsl.isEmpty())
        throw Exception("Invalid hostfile : no default host");

    QRegExp regexDefault("\\S+");
    QRegExp regexLine("(\\S+) (\\d+|(\\d+)\\-(\\d+)|\\*|(\\d+)\\-(\\*)) (\\S+)");

    if (regexDefault.exactMatch(qsl[0]))
    {
        _default = qsl[0];

        if (_default == "localhost" || _default == "local")
            _default = "127.0.0.1";
    }
    else
        throw Exception("Invalid hostfile : invalid first line (a default host must be specified)");

    for (int i = 1; i < qsl.size(); ++i)
    {
        if (!qsl[i].isEmpty())
        {
            if (regexLine.exactMatch(qsl[i]))
            {
                QString module = regexLine.cap(1);
                QString host = regexLine.cap(regexLine.captureCount());

                if (host == "localhost" || host == "local")
                    host = "127.0.0.1";

                if (regexLine.cap(2) == "*") // module * host
                {
                    Set set(0, std::numeric_limits<int>::max());
                    _hosts[module].insert(set, host);
                }
                else if (!regexLine.cap(3).isEmpty()) // module 0-4 host
                {
                    int inf = regexLine.cap(3).toInt();
                    int sup = regexLine.cap(4).toInt();

                    if (inf > sup)
                        throw Exception(QString("Invalid hostfile : in line \"%1\", the first argument must be smaller than the second one").arg(qsl[i]));

                    Set set(inf, sup);
                    _hosts[module].insert(set, host);
                }
                else if (!regexLine.cap(5).isEmpty()) // module 0-* host
                {
                    int inf = regexLine.cap(5).toInt();

                    Set set(inf, std::numeric_limits<int>::max());
                    _hosts[module].insert(set, host);
                }
                else // module 0 host
                {
                    int i = regexLine.cap(2).toInt();

                    Set set(i);
                    _hosts[module].insert(set, host);
                }
            }
            else
                throw Exception(QString("Invalid hostfile : the line \"%1\" is invalid").arg(qsl[i]));
        }
    }

    if (!isValid())
        throw Exception("Invalid hostfile : all ranges related to the same module must be disjoint");
}


