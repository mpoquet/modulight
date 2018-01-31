#ifndef ARGUMENTHANDLER_HPP
#define ARGUMENTHANDLER_HPP

#include <QString>

namespace modulight
{
class ArgumentHandler
{
public:
    enum TaskType
    {
        HOST_FILE = 0x01,
        DOT = 0x02,
        DOT_SIMPLE = 0x04,
        NOTHING = 0x08,
        EXIT = 0x10,
        PARAM_FILE = 0x20
    };

    ArgumentHandler(int argc, char ** argv);

    // Returns a combination of TaskType
    int handle(QString & out1, QString &out2);

private:
    void showHelp();

private:
    QString _program;
    QString _commandLine;
};
}

#endif // ARGUMENTHANDLER_HPP
