#ifndef VARIABLE_H
#define VARIABLE_H

#include <QTextStream>
#include <QObject>
#include <QDebug>
#include <limits>

namespace Langmuir
{

//! A Base class to map between variable names (keys) and locations (references)
class Variable : public QObject
{
    Q_OBJECT
public:
    //! A Flag to alter the behavoir of certain variable member functions
    enum VariableModeFlag
    {
        Constant = 1 //!< When constant, a variable's read / convert function does nothing
    };
    Q_FLAGS(ModeFlag)
    Q_DECLARE_FLAGS(VariableMode, VariableModeFlag)

    //! Create a Variable, see Variable::Variable for description
    Variable(
        const QString &key,
        VariableMode mode = 0,
        QObject *parent = 0);

    //! Cast the value stored in string to the correct type and store it in the correct location
    virtual void read(const QString& token) = 0;    

    //! Get this variable's key (name)
    virtual QString key() const;

    //! Get this variable's value as a QString
    virtual QString value() const = 0;

    //! Get this variable's key and value in the form 'key = value'
    virtual QString keyValue() const;

    //! Operator overload to output 'key = value' to QTextStream
    friend QTextStream& operator<<(QTextStream& stream, const Variable &variable);

    //! Operator overload to output debug information to QDebug
    friend QDebug operator<<(QDebug debug, const Variable &variable);

    //! Write 'key = value' to a stream
    virtual void write(QTextStream& stream) const = 0;

    //! True if the Variable::Constant mode flag was set
    bool isConstant() const;

    //! Get this variable's mode flags
    const VariableMode& mode() const;

    //! Write value to a binary stream
    virtual void binaryWrite(QDataStream& stream) const = 0;

    //! Read value from a binary stream
    virtual void binaryRead(QDataStream& stream) = 0;

protected:

    //! The name of this variable
    QString m_key;

    //! The mode flags for this variable
    VariableMode m_mode;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Variable::VariableMode)

//! A template class to map between variable names (keys) and locations (references)
/*
  The class is a template so that it may point to arbitray data types.  In order to
  add a new data type you must implement the following, with \b 'T' replaced by your
  new type:
    - template <> inline void Variable<T>::convert(const QString& token, T& result)
    - inline QTextStream& operator<<(QTextStream& stream, const T& variable)
  */
template <class T>
class TypedVariable : public Variable
{
public:

    //! Create a new variable
    /*!
      \param key the name of the variable
      \param value the location where the value of the variable is stored
      \param mode flags to alter variable's behavoir, see AbstractVariable::VariableModeFlag
      */
    TypedVariable(
        const QString& key,
        T &value,
        VariableMode mode = 0,
        QObject *parent = 0);

    //! Cast the value stored in string to the correct type and store it in the correct location
    /*!
      This function assumes \b 'QTextStream& operator<<' has been implemented
      for that data type T.  Keep this in mind if adding a new data type.
      */
    virtual void read(const QString& token);

    //! Write 'key = value' to a stream
    virtual void write(QTextStream& stream) const;

    //! A template function for converting a QString to some type T
    /*!
      This was used because things like QVariant and QString.toDouble (etc)
      methods tend to return default constructed values when conversion fails -
      not to mention conversion from string to bool ignores keywords (like true
      or false).  So rather we implement this function for new data types so
      that we can have more control over the conversion and throw errors or
      call qFatal if the conversion fails.  To implement this function for a
      new data type, use declarations of the form:
      \code template <> inline void Variable<T>::convert(const QString& token, T& result)
      Replace the \b 'T' with the data type you want to implement (for example, double).
      */
    static void convert(const QString& token, T& result);

    //! Get this variable's value as a QString
    /*!
      This function assumes \b 'void Variable<T>::convert' has been implemented
      for that data type T.  Keep this in mind if adding a new data type.
      */
    QString value() const;

    //! Write value to a binary stream
    void binaryWrite(QDataStream& stream) const;

    //! Read value from a binary stream
    void binaryRead(QDataStream& stream);

protected:
    T &m_value;
};

// initialize a Variable with a key
inline Variable::Variable(
    const QString &key,
    VariableMode mode,
    QObject *parent)
    : QObject(parent),
      m_key(key),
      m_mode(mode)
{
}

// get the variable's key
inline QString Variable::key() const
{
    return m_key;
}

// get the variable's value (converted to string)
template <class T> inline QString TypedVariable<T>::value() const
{
    QString result; QTextStream stream(&result);
    stream << qSetRealNumberPrecision(std::numeric_limits<T>::digits10)
           << scientific
           << m_value;
    stream.flush();
    return result;
}

// get the variable's value (converted to string)
template <> inline QString TypedVariable<bool>::value() const
{
    QString result; QTextStream stream(&result);
    if (m_value == true) { stream << "true"; }
    else { stream << "false"; }
    stream.flush();
    return result;
}

// get the variable's key
inline QString Variable::keyValue() const
{
    QString result; QTextStream stream(&result);
    stream << left
           << qSetFieldWidth(30)
           << key()
           << qSetFieldWidth(3)
           << '='
           << qSetFieldWidth(0)
           << value();
    stream.flush();
    return result;
}

// see if the Variable is really a Constant
inline bool Variable::isConstant() const
{
    return m_mode.testFlag(Constant);
}

// get the mode flags of this Variable
inline const Variable::VariableMode& Variable::mode() const
{
    return m_mode;
}

// initialize Variable with a key and a location
template <class T> inline TypedVariable<T>::TypedVariable(
    const QString& key,
    T &value,
    VariableMode mode,
    QObject *parent)
    : Variable(key,mode,parent),
      m_value(value)
{
}

// convert a QString token to its correct type
template <class T> inline void TypedVariable<T>::read(const QString& token)
{
    if (m_mode.testFlag(Constant))
    {
        qWarning("ignoring constant; key: %s, token: %s",
                 qPrintable(m_key),qPrintable(token));
        return;
    }
    convert(token,m_value);
}

// Read bytes
template <class T> inline void TypedVariable<T>::binaryRead(QDataStream& stream)
{
    stream >> m_value;
    switch (stream.status())
    {
    case QDataStream::Ok:
    {
        return;
        break;
    }
    case QDataStream::ReadPastEnd:
    {
        QString error = QString("binary stream read error: QDataStream::ReadPastEnd\n\texpected parameter: %1").arg(m_key);

        qFatal("%s",qPrintable(error));
        break;
    }
    case QDataStream::ReadCorruptData:
    {
        QString error = QString("binary stream read error: QDataStream::ReadCorruptData\n\texpected parameter: %1").arg(m_key);
        qFatal("%s",qPrintable(error));
        break;
    }
    }
}

// Write bytes
template <class T> inline void TypedVariable<T>::binaryWrite(QDataStream& stream) const
{
    stream << m_value;
}

// write keyValue() to a stream
template <class T> inline void TypedVariable<T>::write(QTextStream& stream) const
{
    stream << keyValue();
}

// overload operator to write keyValue() to a stream
inline QTextStream& operator<<(QTextStream& stream, const Variable& variable)
{
    variable.write(stream);
    return stream;
}

// overload operator to write keyValue() to a qDebug()
inline QDebug operator<<(QDebug debug, const Variable &variable)
{
    QString result = QString("(%1, %2, %3)")
            .arg(variable.key())
            .arg(variable.value())
            .arg(variable.isConstant());
    return debug << qPrintable(result);
}

// convert to QString
template <> inline void TypedVariable<QString>::convert(const QString& token, QString& result)
{
    result = token.trimmed();
}

// convert to double
template <> inline void TypedVariable<qreal>::convert(const QString& token, qreal& result)
{
    bool ok = false;
    result = token.toDouble(&ok);
    if (!ok) qFatal("can not convert to double: %s", qPrintable(token));
}

// convert to float
template <> inline void TypedVariable<float>::convert(const QString& token, float& result)
{
    bool ok = false;
    result = token.toFloat(&ok);
    if (!ok) qFatal("can not convert to double: %s", qPrintable(token));
}

// convert to bool
template <> inline void TypedVariable<bool>::convert(const QString& token, bool& result)
{
    QString lower = token.trimmed().toLower();
    if (lower == "false")
    {
        result = false;
    }
    else if (lower == "true")
    {
        result = true;
    }
    else
    {
        bool ok = false;
        result = token.toInt(&ok);
        if (!ok) qFatal("can not convert to bool: %s", qPrintable(token));
    }
}

// convert to int
template <> inline void TypedVariable<qint32>::convert(const QString& token, qint32& result)
{
    bool ok = false;
    result = token.toInt(&ok);
    if (!ok) qFatal("can not convert to int: %s", qPrintable(token));
}

// convert to unsigned int
template <> inline void TypedVariable<quint32>::convert(const QString& token, quint32& result)
{
    bool ok = false;
    result = token.toUInt(&ok);
    if (!ok) qFatal("can not convert to unsigned int: %s", qPrintable(token));
}

// convert to long long int
template <> inline void TypedVariable<qint64>::convert(const QString& token, qint64& result)
{
    bool ok = false;
    result = token.toLongLong(&ok);
    if (!ok) qFatal("can not convert to long long int: %s", qPrintable(token));
}

// convert to unsigned long long int
template <> inline void TypedVariable<quint64>::convert(const QString& token, quint64& result)
{
    bool ok = false;
    result = token.toULongLong(&ok);
    if (!ok) qFatal("can not convert to unsigned long long int: %s", qPrintable(token));
}

// convert to QDateTime
template <> inline void TypedVariable<QDateTime>::convert(const QString& token, QDateTime& result)
{
//    bool ok = false;
//    result = token.toULongLong(&ok);
//    if (!ok) qFatal("can not convert to unsigned long long int: %s", qPrintable(token));
}

inline QTextStream& operator<<(QTextStream& stream, const QDateTime& datetime)
{
    return stream;
}

}

#endif // VARIABLE_H
