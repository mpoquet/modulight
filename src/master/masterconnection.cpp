#include <modulight/master/masterconnection.hpp>

bool modulight::MasterConnection::operator ==(const MasterConnection &c)
{
    return  processA == c.processA &&
            portA == c.portA &&
            processB == c.processB &&
            portB == c.portB;
}
