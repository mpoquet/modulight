#include <modulight/module/messagereader.hpp>

#include <iostream>
#include <stdexcept>

using namespace std;

modulight::MessageReader::MessageReader() :
    _data(0),
    _loaded(false),
    _readCursor(0)
{
}

void modulight::MessageReader::load(char *buf, int bufSize, const QString & portName, const QString & source,
                                    int moduleIteration, int portIteration)
{
    if (!_loaded)
    {
        _loaded = true;

        _data.resize(bufSize);
        _readCursor = 0;
        _portName = portName;
        _source = source;
        _moduleIteration = moduleIteration;
        _portIteration = portIteration;

        if (bufSize > 0)
            memcpy(_data.data(), buf, bufSize * sizeof(char));
    }
    else
        cerr << "Bad call of MessageReader::load. This method shouldn't be called by the user" << endl;
}

void modulight::MessageReader::clear()
{
    if (_loaded)
    {
        _loaded = false;

        _data.clear();
        _readCursor = 0;
    }
}

int modulight::MessageReader::readInt()
{
    if (_readCursor + (int)sizeof(int) > _data.size())
        throw Exception("Bad Message::readInt : out of bounds");

    int ret;
    memcpy(&ret, &_data[_readCursor], sizeof(int));
    _readCursor += sizeof(int);

    return ret;
}

float modulight::MessageReader::readFloat()
{
    if (_readCursor + (int)sizeof(float) > _data.size())
        throw Exception("Bad Message::readFloat : out of bounds");

    float ret;
    memcpy(&ret, &_data[_readCursor], sizeof(float));
    _readCursor += sizeof(float);

    return ret;
}

double modulight::MessageReader::readDouble()
{
    if (_readCursor + (int)sizeof(double) > _data.size())
        throw Exception("Bad Message::readDouble : out of bounds");

    double ret;
    memcpy(&ret, &_data[_readCursor], sizeof(double));
    _readCursor += sizeof(double);

    return ret;
}

std::string modulight::MessageReader::readStdString()
{
    if (_readCursor + (int)sizeof(unsigned int) > _data.size())
        throw Exception("Bad Message::readString : out of bounds");

    unsigned int stringSize;

    memcpy(&stringSize, &_data[_readCursor], sizeof(unsigned int));
    _readCursor += sizeof(unsigned int);

    if ((int)(_readCursor + stringSize * sizeof(char)) > _data.size())
        throw Exception("Bad Message::readString : critical internal error in the written string");

    std::string ret(&_data[_readCursor], stringSize);
    _readCursor += stringSize*sizeof(char);

    return ret;
}

QString modulight::MessageReader::readQString()
{
    if (_readCursor + (int)sizeof(unsigned int) > _data.size())
        throw Exception("Bad Message::readString : out of bounds");

    QString ret;
    unsigned int stringSize;

    memcpy(&stringSize, &_data[_readCursor], sizeof(unsigned int));
    _readCursor += sizeof(unsigned int);

    if ((int)(_readCursor + stringSize*sizeof(char)) > _data.size())
        throw Exception("Bad Message::readString : critical internal error in the written string");

    ret = QString::fromUtf8(&_data[_readCursor], stringSize);
    _readCursor += stringSize*sizeof(char);

    return ret;
}

int modulight::MessageReader::cursorPosition() const
{
    return _readCursor;
}

void modulight::MessageReader::setCursorPosition(int cursorPosition)
{
    Q_ASSERT_X(cursorPosition >= 0 && cursorPosition < _data.size(), "MessageReader::setCursorPosition", "Invalid cursor position");
    _readCursor = cursorPosition;
}

const char *modulight::MessageReader::data() const
{
    return _data.data();
}

int modulight::MessageReader::size() const
{
     return _data.size();
}
