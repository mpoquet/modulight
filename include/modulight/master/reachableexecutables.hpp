#ifndef REACHABLEEXECUTABLES_HPP
#define REACHABLEEXECUTABLES_HPP

#include <QStringList>

namespace modulight
{
/**
 * @brief Lists reachable executables and finds their absolute filename
 */
class ReachableExecutables
{
public:
    ReachableExecutables();

    /**
     * @brief Updates the reachable executables list
     */
    void update();

    /**
     * @brief Allows to know whether a given command is reachable or not
     * @param command The command
     * @return true if the command can be executed, false otherwise
     */
    bool contains(const QString & command);

    /**
     * @brief Returns the absolute filename of a command
     * @param command The command
     * @return The corresponding absolute filename
     */
    QString absoluteFilename(const QString & command);

private:
    QStringList _executables;
};
}

#endif // REACHABLEEXECUTABLES_HPP
