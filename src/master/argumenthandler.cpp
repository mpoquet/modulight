#include <modulight/master/argumenthandler.hpp>

#include <iostream>

#include <QDebug>
#include <QString>
#include <QStringList>

using std::cout;
using std::endl;

modulight::ArgumentHandler::ArgumentHandler(int argc, char **argv)
{
    if (argc < 1)
    {
        qDebug() << "Strange argument count, it should be 1 or more";
        return;
    }

    _program = argv[0];
    _program = _program.split("/").last();

    if (argc > 1)
    {
        _commandLine = argv[1];

        for (int i = 2; i < argc; ++i)
            _commandLine += ' ' + QString(argv[i]);
    }
}

int modulight::ArgumentHandler::handle(QString & out1, QString & out2)
{
    if (!_commandLine.isEmpty())
    {
        QRegExp regexHelp("--help|-h$|-h ");
        QRegExp regexDot("--dot (\\S+)");
        QRegExp regexDotSimple("--dots (\\S+)");
        QRegExp regexHostfile("--host (\\S+)");
        QRegExp regexParamfile("--args (\\S+)");

        if (_commandLine.contains(regexHelp))
        {
            showHelp();
            return EXIT;
        }
        else if (_commandLine.contains(regexDot))
        {
            out1 = regexDot.cap(1);

            return DOT;
        }
        else if (_commandLine.contains(regexDotSimple))
        {
            out1 = regexDotSimple.cap(1);

            return DOT_SIMPLE;
        }
        else
        {
            bool didSomething = false;
            int ret = 0;

            if (_commandLine.contains(regexHostfile))
            {
                out1 = regexHostfile.cap(1);
                ret |= HOST_FILE;
                didSomething = true;
            }

            if (_commandLine.contains(regexParamfile))
            {
                out2 = regexParamfile.cap(1);
                ret |= PARAM_FILE;
                didSomething = true;
            }

            if (didSomething)
                return ret;
            else
            {
                cout << QString("%1: unrecognized option").arg(_program).toStdString() << endl << endl << endl;
                showHelp();
                return EXIT;
            }
        }
    }

    return NOTHING;
}

void modulight::ArgumentHandler::showHelp()
{
    QString help = QString("usage: mpirun %1 [OPTION]").arg(_program);

    cout << help.toStdString() << endl;
    cout << endl;
    cout << " -h, --help        shows this help" << endl;
    cout << " --dot FILE        instead of running the application, writes a dot" << endl;
    cout << "                   description of the static application in FILE" << endl;
    cout << " --dots FILE       does the same as --dot, but in a simplified version" << endl;
    cout << " --host FILE       loads FILE as the application host file" << endl;
    cout << " --args FILE       set FILE as the application argument file" << endl;
}
