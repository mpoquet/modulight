#ifndef MESSAGEREADER_HPP
#define MESSAGEREADER_HPP

#include <string>
#include <vector>
#include <stdexcept>

#include <QSharedPointer>
#include <QString>
#include <QVector>

#include <modulight/common/modulightexception.hpp>

namespace modulight
{
class Module;
/**
 * \addtogroup groupModule
 * @{
 */
/**
 * @brief Easy-to-use class to read a message
 *
 * This class provides a way to read a message, thanks to Module::readMessage(QString, MessageReader&).<br/>
 * To avoid performance issues, no endianness conversion is done on data.<br/>
 * <br/>
 * All read functions extract a value from the data and then moves the read cursor.<br/>
 * This cursor might be obtained via cursorPosition() and moved with setCursorPosition(int)
 */
class MessageReader
{
    friend class modulight::Module;
public:
    /**
     * @brief Constructor
     */
    MessageReader();

    /** @name Data access methods
     * These methods allow to access the data stored in the MessageReader
     */
    ///@{

    /**
    * @brief Gets the data pointer
    * @return The data pointer
    */
   const char* data() const;

   /**
    * @brief Gets the message size
    * @return The message size, in bytes
    */
   int size() const;

    /**
     * @brief Reads an integer
     * @return The read integer
     *
     * This method reads an integer and adds sizeof(int) to the read cursor
     */
    int readInt();

    /**
     * @brief Reads a float
     * @return The read float
     *
     * This method reads a float and adds sizeof(float) to the read cursor
     */
    float readFloat();

    /**
     * @brief Reads a double
     * @return The read double
     *
     * This method reads a double and adds sizeof(double) to the read cursor
     */
    double readDouble();

    /**
     * @brief Reads a std::string
     * @return The read std::string
     *
     * This method reads an integer <i>size</i> (the string size) then reads <i>size</i> characters.<br/>
     * This method adds sizeof(int) + <i>size</i> to the read cursor
     */
    std::string readStdString();

    /**
     * @brief Reads a QString, assuming it had been sent in UTF-8
     * @return The read QString
     *
     * This method reads an integer <i>size</i> (the string size) then reads <i>size</i> characters.<br/>
     * This method adds sizeof(int) + <i>size</i> to the read cursor
     */
    QString readQString();

    /**
     * @brief Reads a std::vector<T>
     * @return The std::vector<T>
     *
     * This method reads an integer <i>size</i> (the vector size) then reads <i>size</i> T.<br/>
     * This method adds sizeof(int) + <i>size</i> * sizeof(T) to the read cursor.<br/>
     * Please ensure that T can be copied via memcpy
     */
    template<typename T>
    std::vector<T> readStdVector();

    /**
     * @brief Reads a QVector<T>
     * @return The read QVector<T>
     *
     * This method reads an integer <i>size</i> (the vector size) then reads <i>size</i> T.<br/>
     * This method adds sizeof(int) + <i>size</i> * sizeof(T) to the read cursor.<br/>
     * Please ensure that T can be copied via memcpy
     */
    template<typename T>
    QVector<T> readQVector();

    /**
     * @brief Reads a T
     * @return The read T
     *
     * Please ensure that T can be copied via memcpy
     */
    template<typename T>
    T read();

    /**
     * @brief Gets the cursor position
     * @return The cursor position, in bytes
     */
    int cursorPosition() const;

    /**
     * @brief Sets the cursor position
     * @param cursorPosition The new cursor position
     *
     * Please note that the new cursor position must be valid (0 <= cursorPosition < size())
     */
    void setCursorPosition(int cursorPosition);

    ///@}

    /** @name Metadata access methods
     * These methods allow to access the metadata stored in the MessageReader
     */
    ///@{

    /**
     * @brief Gets the local input port name on which the message had been received
     * @return The local input port name on which the message had been received
     */
    QString localPortName() const { return _portName; }

    /**
     * @brief Gets the source of the received message
     * @return The message source, as a QString formatted "Foo42:out", which means the output port <i>out</i> of the 42th instance of the module Foo
     */
    QString sourceName() const { return _source; }

    /**
     * @brief Gets the iteration number of the process which emitted the message
     * @return The iteration number of the process which emitted the message
     */
    int sourceProcessIterationNumber() const { return _moduleIteration; }

    /**
     * @brief Gets the iteration number of the output port on which the message had been sent
     * @return The iteration number of the output port on which the message had been sent
     */
    int sourcePortIterationNumber() const { return _portIteration; }

    ///@}

private:
    void load(char * buf, int bufSize, const QString & localPortName, const QString & sourceName, int sourceProcessIterationNumber, int sourcePortIterationNumber);
    void clear();

private:
    QVector<char> _data;
    bool _loaded;
    int _readCursor;

    QString _portName;
    QString _source;
    int _moduleIteration;
    int _portIteration;
};
/// @}
}

template<typename T>
std::vector<T> modulight::MessageReader::readStdVector()
{
    if (_readCursor + (int)sizeof(unsigned int) > _data.size())
        throw Exception("Bad Message::readVector : out of bounds");

    unsigned int vectorSize;

    memcpy(&vectorSize, &_data[_readCursor], sizeof(unsigned int));
    _readCursor += sizeof(unsigned int);

    if (_readCursor + (int)(vectorSize*sizeof(T)) > _data.size())
        throw Exception("Bad Message::readVector : critical internal error in the written string");

    std::vector<T> ret (vectorSize);
    memcpy(ret.data(), &_data[_readCursor], vectorSize * sizeof(T));
    _readCursor += vectorSize * sizeof(T);

    return ret;
}

template<typename T>
QVector<T> modulight::MessageReader::readQVector()
{
    if (_readCursor + (int)sizeof(unsigned int) > _data.size())
        throw Exception("Bad Message::readVector : out of bounds");

    unsigned int vectorSize;

    memcpy(&vectorSize, &_data[_readCursor], sizeof(unsigned int));
    _readCursor += sizeof(unsigned int);

    if (_readCursor + (int)(vectorSize*sizeof(T)) > _data.size())
        throw Exception("Bad Message::readVector : critical internal error in the written string");

    QVector<T> ret(vectorSize);
    memcpy(ret.data(), &_data[_readCursor], vectorSize * sizeof(T));
    _readCursor += vectorSize * sizeof(T);

    return ret;
}

template<typename T>
T modulight::MessageReader::read()
{
    if (_readCursor + (int)sizeof(T) > _data.size())
        throw Exception("Bad Message::read : out of bounds");

    T ret;

    memcpy(&ret, &_data[_readCursor], sizeof(T));
    _readCursor += sizeof(T);

    return ret;
}

#endif // MESSAGEREADER_HPP
