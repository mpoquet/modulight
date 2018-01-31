#ifndef MESSAGEWRITER_HPP
#define MESSAGEWRITER_HPP

#include <string>
#include <vector>

#include <QSharedPointer>
#include <QVector>

namespace modulight
{
/**
 * \addtogroup groupModule
 * @{
 */
/**
 * @brief Allows to create a user-formatted message
 *
 * This class might be used to create a message buffer, which can be send with the Module::send(const QString &, const MessageWriter &) method.
 */
class MessageWriter
{
public:
    /**
     * @brief Constructor
     * @param reserveSize The initial size to reserve for the message buffer
     *
     * To avoid data copies, giving a reserveSize corresponding to the message final size is better.<br/>
     * However, the buffer will be resized when needed.
     */
    MessageWriter(int reserveSize = 0);

    /** @name Data writing methods
     * These methods allow to append data into in the message buffer
     */
    ///@{

    /**
     * @brief Appends an integer at the end of the message buffer
     * @param i The integer
     */
    void writeInt(int i);

    /**
     * @brief Appends a float at the end of the message buffer
     * @param f The float
     */
    void writeFloat(float f);

    /**
     * @brief Appends a double at the end of the message buffer
     * @param d The double
     */
    void writeDouble(double d);

    /**
     * @brief Appends a std::string at the end of the message buffer
     * @param str The std::string
     *
     * The string size is written first as an integer.<br/>
     * The string content is then written, without the final character '\0'
     */
    void writeStdString(const std::string & str);

    /**
     * @brief Appends a std::string at the end of the message buffer
     * @param str The QString
     *
     * Firstly, the QString if converted to an UTF-8 <i>representation</i> of it<br/>
     * The <i>representation</i> string size is written first as an integer.<br/>
     * The <i>representation</i> content is then written, without the final character '\0'
     */
    void writeQString(const QString & str);

    /**
     * @brief Appends raw data at the end of the message buffer
     * @param ptr The data pointer
     * @param size The number of bytes to write
     */
    void writeData(const void *ptr, unsigned int size);

    /**
     * @brief Appends a std::vector<T> at the end of the message buffer
     * @param v The std::vector<T>
     *
     * The vector size is written first as an integer.<br/>
     * The vector content is then copied at the end of the message buffer.<br/>
     * <br/>
     * Please ensure that your data can be copied with memcpy
     */
    template<typename T>
    void writeStdVector(const std::vector<T> & v);

    /**
     * @brief Appends a QVector<T> at the end of the message buffer
     * @param v The QVector<T>
     *
     * The vector size is written first as an integer.<br/>
     * The vector content is then copied at the end of the message buffer.<br/>
     * <br/>
     * Please ensure that your data can be copied with memcpy
     */
    template<typename T>
    void writeQVector(const QVector<T> & v);

    /**
     * @brief Appends a T at the end of the message buffer
     * @param t The T
     *
     * Please ensure that your data can be copied with memcpy
     */
    template<typename T>
    void write(const T & t);

    ///@}

    /**
     * @brief Gets the message buffer pointer
     * @return The message buffer pointer
     */
    const char* data() const;

    /**
     * @brief Gets the message buffer size
     * @return The message buffer size, in bytes
     */
    int size() const { return _data->size(); }

private:
    QSharedPointer<QVector<char> > _data;
};
/// @}
}

template<typename T>
void modulight::MessageWriter::writeStdVector(const std::vector<T> & v)
{
    int previousDataSize = _data->size();
    unsigned int vectorSize = v.size();

    _data->resize(previousDataSize + sizeof(unsigned int) + vectorSize * sizeof(T));

    memcpy(&(*_data)[previousDataSize], &vectorSize, sizeof(unsigned int));
    memcpy(&(*_data)[previousDataSize + sizeof(unsigned int)], v.data(), vectorSize * sizeof(T));
}

template<typename T>
void modulight::MessageWriter::writeQVector(const QVector<T> & v)
{
    int previousDataSize = _data->size();
    unsigned int vectorSize = v.size();

    _data->resize(previousDataSize + sizeof(unsigned int) + vectorSize * sizeof(T));
    memcpy(&(*_data)[previousDataSize], &vectorSize, sizeof(unsigned int));
    memcpy(&(*_data)[previousDataSize + sizeof(unsigned int)], v.data(), vectorSize * sizeof(T));
}

template<typename T>
void modulight::MessageWriter::write(const T & t)
{
    int previousDataSize = _data->size();

    _data->resize(previousDataSize + sizeof(T));
    memcpy(&(*_data)[previousDataSize], &t, sizeof(T));
}

#endif // MESSAGEWRITER_HPP
