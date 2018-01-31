#ifndef STAMP_HPP
#define STAMP_HPP

#include <QString>

namespace modulight
{
class Stamp
{
public:
    Stamp();
    Stamp(bool realMessage, const QByteArray * source, int moduleIteration,
          int portIteration, int userDataSize = 0, void * userData = 0);
    Stamp(const Stamp & other);
    Stamp & operator=(const Stamp & other);

    ~Stamp();

    int size() const { return _size; }
    char * data() const { return _data; }

    bool isReal() const { return _realMessage == 1; }
    int moduleIteration() const { return _moduleIteration; }
    int portIteration() const { return _portIteration; }

    static void extractUsefulInformationFromData(char * data,
                                                 int & moduleIteration, int & portIteration,
                                                 bool & isReal, QString & source);

private:
    int calculateSize();
    void copyDataTo(char * dest);

private:
    int _moduleIteration;
    int _portIteration;
    int _realMessage; //1 if the message is real, 0 otherwise

    const QByteArray * _source;

    int _userDataSize;
    void * _userData;

    char * _data;
    int _size;
};
}

#endif // STAMP_HPP
