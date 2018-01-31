#include <modulight/module/stamp.hpp>

#include <cstdlib>

modulight::Stamp::Stamp()
{
    _moduleIteration = -1;
    _portIteration = -1;
    _realMessage = 0;

    QByteArray qba = QString("no_source").toUtf8();
    _source = &qba;

    _userDataSize = 0;
    _userData = 0;

    _size = calculateSize();
    _data = (char*)malloc(_size);
    copyDataTo(_data);
}

modulight::Stamp::Stamp(bool realMessage, const QByteArray *source, int moduleIteration,
                        int portIteration, int userDataSize, void *userData) :
    _moduleIteration(moduleIteration),
    _portIteration(portIteration),
    _source(source),
    _userDataSize(userDataSize),
    _userData(userData)
{
    if (realMessage)
        _realMessage = 1;
    else
        _realMessage = 0;

    _size = calculateSize();
    _data = (char*)malloc(_size);
    copyDataTo(_data);
    // todo : handle user data (maybe copy ?)
}

modulight::Stamp::Stamp(const modulight::Stamp &other)
{
    _moduleIteration = other._moduleIteration;
    _portIteration = other._portIteration;
    _realMessage = other._realMessage;
    _source = other._source;
    _userDataSize = other._userDataSize;
    _userData = other._userData; // real copy ? sharedptr ?

    _size = other._size;
    _data = (char*)malloc(_size); // todo : shared pointer to avoid copy ?
    memcpy(_data, other._data, _size);
}

modulight::Stamp &modulight::Stamp::operator =(const modulight::Stamp &other)
{
    free(_data);

    _moduleIteration = other._moduleIteration;
    _portIteration = other._portIteration;
    _realMessage = other._realMessage;
    _source = other._source;
    _userDataSize = other._userDataSize;
    _userData = other._userData; // real copy ? sharedptr ?

    _size = other._size;
    _data = (char*)malloc(_size); // todo : shared pointer to avoid copy ?
    memcpy(_data, other._data, _size);

    return *this;
}

modulight::Stamp::~Stamp()
{
    free(_data);
}

void modulight::Stamp::extractUsefulInformationFromData(char *data, int &moduleIteration, int &portIteration,
                                                        bool &isReal, QString &source)
{
    char * dest = data;

    memcpy(&moduleIteration, dest, sizeof(int));
    dest += sizeof(int);

    memcpy(&portIteration, dest, sizeof(int));
    dest += sizeof(int);


    int isRealInt;
    memcpy(&isRealInt, dest, sizeof(int));
    isReal = isRealInt;
    dest += sizeof(int);


    int sourceSize;
    memcpy(&sourceSize, dest, sizeof(int));
    dest += sizeof(int);

    source = QString::fromUtf8(dest, sourceSize);
    dest += sourceSize;

    // todo : handle user data
}

int modulight::Stamp::calculateSize()
{
    return sizeof(int)          // module iteration
            + sizeof(int)       // port iteration
            + sizeof(int)       // real message
            + sizeof(int)       // source size
            + _source->size()   // source data
            + sizeof(int)       // user data size
            + _userDataSize;    // user data
}

void modulight::Stamp::copyDataTo(char *dest)
{
    int sourceSize = _source->size();

    memcpy(dest, &_moduleIteration, sizeof(int));
    dest += sizeof(int);

    memcpy(dest, &_portIteration, sizeof(int));
    dest += sizeof(int);

    memcpy(dest, &_realMessage, sizeof(int));
    dest += sizeof(int);


    memcpy(dest, &sourceSize, sizeof(int));
    dest += sizeof(int);

    memcpy(dest, _source->data(), sourceSize);
    dest += sourceSize;


    memcpy(dest, &_userDataSize, sizeof(int));
    dest += sizeof(int);

    memcpy(dest, _userData, _userDataSize);
}
