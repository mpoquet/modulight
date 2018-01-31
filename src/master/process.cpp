#include <modulight/master/process.hpp>

modulight::Process::Process()
{
    canAlterNetwork = false;
    isFinished = false;
    isAboutToBeFinished = false;
}

bool modulight::Process::operator ==(const Process & p) const
{
    return id == p.id;
}

bool modulight::Process::operator <(const Process &p) const
{
    return id < p.id;
}
