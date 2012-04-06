#ifndef VARIABLE_H
#define VARIABLE_H

#include <QTextStream>
#include <QDateTime>
#include <QObject>
#include <QDebug>
#include <limits>
#include <ostream>

namespace Langmuir
{

//! A class to map between variable names (keys) and locations (references)
/*
  The class is a template so that it may point to arbitray data types.  In order to
  add a new data type you must implement the following, with \b 'T' replaced by your
  new type:
    - template <> inline void Variable<T>::convert(const QString& token, T& result)
    - inline QTextStream& operator<<(QTextStream& stream, const T& variable)
  */
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
    /*!
      This function assumes \b 'QTextStream& operator<<' has been implemented
      for that data type T.  Keep this in mind if adding a new data type.
      */
    virtual void read(const QString& token) = 0;    

    //! Get this variable's key (name)
    virtual QString key() const = 0;

    //! Get this variable's value as a QString
    virtual QString value() const = 0;

    //! Get this variable's key and value in the form 'key = value'
    virtual QString keyValue() const = 0;

    //! True if the Variable::Constant mode flag was set
    bool isConstant() const;

    //! Get this variable's mode flags
    const VariableMode& mode() const;

    //! Operator overload to output 'key = value' to QTextStream
    friend QTextStream& operator<<(QTextStream& stream, const Variable &variable);

    //! Operator overload to output to output 'key = value' to std::ofstream
    friend std::ostream& operator<<(std::ostream &stream, Variable &variable);

protected:

    //! The name of this variable
    QString m_key;

    //! The mode flags for this variable
    VariableMode m_mode;

    //! Write 'key = value' to a stream
    virtual void write(QTextStream& stream) const = 0;

};
Q_DECLARE_OPERATORS_FOR_FLAGS(Variable::VariableMode)

//! A template class to map between variable names (keys) and locations (references)
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
    virtual void read(const QString& token);

    //! Get this variable's key (name)
    virtual QString key() const;

    //! Get this variable's value as a QString
    virtual QString value() const;

    //! Get this variable's key and value in the form 'key = value'
    virtual QString keyValue() const;

    //! A template function for converting a QString to some type T
    /*!
      To implement this function for a new data type, use declarations of the form:
       - template <> inline void Variable<T>::convert(const QString& token, T& result)
       - replace the \b 'T' with the data type you want to implement (for example, double).
      */
    static void convert(const QString& token, T& result);

protected:
    T &m_value;

    //! Write 'key = value' to a stream
    virtual void write(QTextStream& stream) const;
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
template <class T> inline QString TypedVariable<T>::key() const
{
    return m_key;
}

// get the variable's value (converted to string)
template <class T> inline QString TypedVariable<T>::value() const
{
    QString result; QTextStream stream(&result);
    stream << m_value;
    stream.flush();
    return result;
}

// get the variable's value (converted to string)
template <> inline QString TypedVariable<float>::value() const
{
    QString result; QTextStream stream(&result);
    stream << qSetRealNumberPrecision(std::numeric_limits<float>::digits10)
           << scientific
           << m_value;
    stream.flush();
    return result;
}

// get the variable's value (converted to string)
template <> inline QString TypedVariable<qreal>::value() const
{
    QString result; QTextStream stream(&result);
    stream << qSetRealNumberPrecision(std::numeric_limits<qreal>::digits10)
           << scientific
           << m_value;
    stream.flush();
    return result;
}

// get the variable's value (converted to string)
template <> inline QString TypedVariable<bool>::value() const
{
    QString result; QTextStream stream(&result);
    if (m_value == true) { stream << "True"; }
    else { stream << "False"; }
    stream.flush();
    return result;
}

// get the variable's key
template <class T> inline QString TypedVariable<T>::keyValue() const
{
    QString result; QTextStream stream(&result);
    stream << left
           << qSetFieldWidth(30)
           << key();
    QString theValue = value();
    if (theValue.startsWith('-'))
    {
        stream << qSetFieldWidth(2)
               << '='
               << qSetFieldWidth(0)
               << theValue;
    }
    else
    {
        stream << qSetFieldWidth(3)
               << '='
               << qSetFieldWidth(0)
               << theValue;
    }
    stream.flush();
    return result;
}

// get the variable's key
template <> inline QString TypedVariable<QString>::keyValue() const
{
    QString result; QTextStream stream(&result);
    stream << left
           << qSetFieldWidth(30)
           << key();
    if (m_value.isEmpty())
    {
        stream << qSetFieldWidth(0)
               << '=';
    }
    else
    {
        stream << qSetFieldWidth(3)
               << '='
               << qSetFieldWidth(0)
               << value();
    }
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

//! Operator overload to output to output 'key = value' to std::ostream
inline std::ostream& operator<<(std::ostream &stream, Variable &variable)
{
    stream << variable.keyValue().toStdString();
    return stream;
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

// convert to QDateTime, assumes data is a qint64 mSecsSinceEpoch
template <> inline void TypedVariable<QDateTime>::convert(const QString& token, QDateTime& result)
{
    bool ok = false;
    qint64 msecSinceEpoch = token.toLongLong(&ok);
    if (!ok) qFatal("can not convert to QDateTime (expecting long long int): %s", qPrintable(token));
    result = QDateTime::fromMSecsSinceEpoch(msecSinceEpoch);
}

// output QDateTime as qint64 mSecsSinceEpoch
inline QTextStream& operator<<(QTextStream& stream, const QDateTime& datetime)
{
    stream << datetime.toMSecsSinceEpoch();
    return stream;
}

}

#endif // VARIABLE_H
