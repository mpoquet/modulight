#ifndef ARGUMENTS_HPP
#define ARGUMENTS_HPP

#include <QString>
#include <QMap>
#include <QVariant>

namespace modulight
{
/**
 * @brief Base class for reading and writing process arguments
 */
class Arguments
{
public:
    void addString(const QString & arg, const QString & value);
    void addInt(const QString & arg, int value);
    void addFloat(const QString & arg, float value);
    void addDouble(const QString & arg, double value);
    void addBool(const QString & arg, bool value);

    QString getString(const QString & arg, const QString & defaultValue) const;
    int getInt(const QString & arg, int defaultValue) const;
    float getFloat(const QString & arg, float defaultValue) const;
    double getDouble(const QString & arg, double defaultValue) const;
    bool getBool(const QString & arg, bool defaultValue) const;

    bool isEmpty() const;

    QString toXML() const;
    void loadFromXML(const QString & xmlString);

    const QMap<QString, QString> & map() const { return _map; }
    void setMap(const QMap<QString, QString> & map) { _map = map; }

private:
    QMap<QString, QString> _map;
};

/**
 * @brief Allows to write process arguments
 */
class ArgumentWriter
{
public:
    /**
     * @brief Adds a string argument
     * @param arg The argument name
     * @param value The argument value
     */
    void addString(const QString & arg, const QString & value) { _args.addString(arg, value); }

    /**
     * @brief Adds an integer argument
     * @param arg The argument name
     * @param value The argument value
     */
    void addInt(const QString & arg, int value) { _args.addInt(arg, value); }

    /**
     * @brief Adds a float argument
     * @param arg The argument name
     * @param value The argument value
     */
    void addFloat(const QString & arg, float value) { _args.addFloat(arg, value); }

    /**
     * @brief Adds a double argument
     * @param arg The argument name
     * @param value The argument value
     */
    void addDouble(const QString & arg, double value) { _args.addDouble(arg, value); }

    /**
     * @brief Adds a bool argument
     * @param arg The argument name
     * @param value The argument value
     */
    void addBool(const QString & arg, bool value) { _args.addBool(arg, value); }

    /**
     * @brief Allows to know whether an ArgumentWriter is empty or not
     * @return true if the ArgumentWriter doesn't contain any argument, false otherwise
     */
    bool isEmpty() const { return _args.isEmpty(); }

    /**
     * @brief Converts an ArgumentWriter to its XML representation
     * @return The XML representation of the ArgumentWriter
     */
    QString toXML() const { return _args.toXML(); }

    /**
     * @brief Gets the arguments map, which associates argument names to their value (all values are in QString)
     * @return The arguments map
     */
    const QMap<QString, QString> & map() const { return _args.map(); }

    /**
     * @brief Sets the internal map to the given one. The user shouldn't use this method
     * @param map The given map
     */
    void setMap(const QMap<QString, QString> & map) { _args.setMap(map); }

private:
    Arguments _args;
};

/**
 * @brief Allows to read process arguments
 */
class ArgumentReader
{
public:
    /**
     * @brief Reads a string argument
     * @param arg The argument name
     * @param defaultValue The argument default value, which will be used if the given parameter wasn't set when building the application up
     * @return The argument value
     */
    QString getString(const QString & arg, const QString & defaultValue) const { return _args.getString(arg, defaultValue); }

    /**
     * @brief Reads an integer argument
     * @param arg The argument name
     * @param defaultValue The argument default value, which will be used if the given parameter wasn't set when building the application up
     * @return The argument value
     */
    int getInt(const QString & arg, int defaultValue) const { return _args.getInt(arg, defaultValue); }

    /**
     * @brief Reads a float argument
     * @param arg The argument name
     * @param defaultValue The argument default value, which will be used if the given parameter wasn't set when building the application up
     * @return The argument value
     */
    float getFloat(const QString & arg, float defaultValue) const { return _args.getFloat(arg, defaultValue); }

    /**
     * @brief Reads a double argument
     * @param arg The argument name
     * @param defaultValue The argument default value, which will be used if the given parameter wasn't set when building the application up
     * @return The argument value
     */
    double getDouble(const QString & arg, double defaultValue) const { return _args.getDouble(arg, defaultValue); }

    /**
     * @brief Reads a bool argument
     * @param arg The argument name
     * @param defaultValue The argument default value, which will be used if the given parameter wasn't set when building the application up
     * @return The argument value
     */
    bool getBool(const QString & arg, bool defaultValue) const { return _args.getBool(arg, defaultValue); }

    /**
     * @brief Loads an ArgumentReader from its XML representation
     * @param xmlString The XML representation
     */
    void loadFromXML(const QString & xmlString) { _args.loadFromXML(xmlString); }

private:
    Arguments _args;
};
}

#endif // ARGUMENTS_HPP
