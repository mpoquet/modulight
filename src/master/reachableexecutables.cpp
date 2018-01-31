#include <modulight/master/reachableexecutables.hpp>

#include <stdexcept>

#include <QProcessEnvironment>
#include <QDir>

#include <modulight/common/modulightexception.hpp>

modulight::ReachableExecutables::ReachableExecutables()
{
    update();
}

void modulight::ReachableExecutables::update()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString envPath;
    if (env.contains("MODULIGHT_PATH"))
        envPath = env.value("MODULIGHT_PATH", ".");

    QStringList pathDirs = envPath.split(":");

    for (int i = 0; i < pathDirs.size(); ++i)
    {
        QDir dir(pathDirs[i]);
        _executables += dir.entryList(QDir::Executable | QDir::Files);
    }
}

bool modulight::ReachableExecutables::contains(const QString &command)
{
    return _executables.contains(command);
}

QString modulight::ReachableExecutables::absoluteFilename(const QString &command)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString envPath;
    if (env.contains("MODULIGHT_PATH"))
        envPath = env.value("MODULIGHT_PATH");

    QStringList pathDirs = envPath.split(":");

    for (int i = 0; i < pathDirs.size(); ++i)
    {
        QDir dir(pathDirs[i]);

        if (dir.entryList(QDir::Executable | QDir::Files).contains(command))
            return dir.absoluteFilePath(command);
    }

    throw Exception(QString("Invalid ReachableExecutables::absoluteFilename call : no such command accessible(%1)").arg(command));
}
