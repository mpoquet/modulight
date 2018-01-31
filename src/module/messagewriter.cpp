#include <modulight/module/messagewriter.hpp>

modulight::MessageWriter::MessageWriter(int reserveSize) :
    _data(new QVector<char>)
{
    _data->reserve(reserveSize);
}

void modulight::MessageWriter::writeInt(int i)
{
    int previousDataSize = _data->size();

    _data->resize(previousDataSize + sizeof(int));
    memcpy(&(*_data)[previousDataSize], &i, sizeof(int));
}

void modulight::MessageWriter::writeFloat(float f)
{
    int previousDataSize = _data->size();

    _data->resize(previousDataSize + sizeof(float));
    memcpy(&(*_data)[previousDataSize], &f, sizeof(float));
}

void modulight::MessageWriter::writeDouble(double d)
{
    int previousDataSize = _data->size();

    _data->resize(previousDataSize + sizeof(double));
    memcpy(&(*_data)[previousDataSize], &d, sizeof(double));
}

void modulight::MessageWriter::writeStdString(const std::string &str)
{
    int previousDataSize = _data->size();
    unsigned int strSize = str.size();

    _data->resize(previousDataSize + sizeof(unsigned int) + strSize * sizeof(char));
    memcpy(&(*_data)[previousDataSize], &strSize, sizeof(unsigned int));
    memcpy(&(*_data)[previousDataSize + sizeof(unsigned int)], str.c_str(), strSize * sizeof(char));
}

void modulight::MessageWriter::writeQString(const QString &str)
{
    QByteArray qba = str.toUtf8();
    int previousDataSize = _data->size();
    unsigned int strSize = qba.size();

    _data->resize(previousDataSize + sizeof(unsigned int) + strSize * sizeof(char));
    memcpy(&(*_data)[previousDataSize], &strSize, sizeof(unsigned int));
    memcpy(&(*_data)[previousDataSize + sizeof(unsigned int)], qba.data(), strSize * sizeof(char));
}

void modulight::MessageWriter::writeData(const void * ptr, unsigned int size)
{
    int previousDataSize = _data->size();

    _data->resize(previousDataSize + size);
    memcpy(&(*_data)[previousDataSize], ptr, size);
}

const char *modulight::MessageWriter::data() const
{
    return (*_data).data();
}
