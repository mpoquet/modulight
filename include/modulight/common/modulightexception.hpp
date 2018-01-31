#ifndef MODULIGHT_EXCEPTION_HPP
#define MODULIGHT_EXCEPTION_HPP

#include <stdexcept>

#include <QByteArray>
#include <QString>

namespace modulight
{

class Exception
{
public:
    Exception(const QString & message) { _message = message.toUtf8(); }
    virtual const char * what() const { return _message.data(); }

private:
    QByteArray _message;
};

class DestroyOrderReceivedException : public Exception
{
public:
    DestroyOrderReceivedException(const QString & message) : Exception(message){}
};
}

#endif
