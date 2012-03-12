#ifndef PARSER_H
#define PARSER_H

#include <QObject>

#include "boost/program_options.hpp"

#include "parameters.h"

//! support for boost::program_options to handle QString
std::istream& operator>>(std::istream& stream, QString& string);

namespace Langmuir
{

//! class to help print variable name and value
class AbstractVariable
{
public:
    AbstractVariable(const QString& name)
        : m_name(name) {}
    friend QTextStream& operator<<(QTextStream& stream, const AbstractVariable& variable);
protected:
    QString m_name;
    virtual void write(QTextStream &stream) const = 0;
};

//! class to help print variable name and value
template <class T>
class Variable : public AbstractVariable
{
public:
    Variable(const QString& name, const T &value)
        : AbstractVariable(name), m_value(value) {}
protected:
    const T &m_value;
    void write(QTextStream &stream) const;
};

//! class to parse the command line and a configuration file
class Parser : public QObject
{
    Q_OBJECT
public:
    explicit Parser(QObject *parent = 0);
    ~Parser();
    void parse(const QString& fileName = "");
    void parse(int &argc, char **argv);
    SimulationParameters& parameters();
    void saveParameters();

private:

    void describeParameters();
    template <typename T> void createVariable(const QString& name, T &location, const QString desciption = "");

    boost::program_options::positional_options_description m_description_pos;
    boost::program_options::options_description            m_description;
    boost::program_options::variables_map                  m_variables;
    QList<AbstractVariable*>                               m_printable;

    SimulationParameters m_parameters;
};

//! function to allow boost::program_options to parse into QString
void validate(boost::any& v, const std::vector<std::string>& values, QString*, int);

//! function to write key=value to a QTextStream
inline QTextStream& operator<<(QTextStream& stream, const AbstractVariable& variable)
{
    variable.write(stream);
    return stream;
}

//! function to write key=value to a QTextStream
template <class T> inline void Variable<T>::write(QTextStream &stream) const
{
    stream << left   << qSetFieldWidth(30) << m_name
           << center << qSetFieldWidth( 3) << '='
           << left   << qSetFieldWidth( 0) << m_value
           << right  << qSetFieldWidth( 0) << '\n';
}

//! function to write key=value to a QTextStream for bool
template <> inline void Variable<bool>::write(QTextStream &stream) const
{
    stream << left   << qSetFieldWidth(30) << m_name
           << center << qSetFieldWidth( 3) << '='
           << left   << qSetFieldWidth( 0);
    if (m_value)
    {
        stream << "true";
    }
    else
    {
        stream << "false";
    }
    stream << right  << qSetFieldWidth( 0) << '\n';
}

//! function to add a new variable, with a certain location, and help message
template <typename T> void Parser::createVariable(const QString& name, T &location, const QString desciption)
{
    m_description.add_options()
            (name.toLocal8Bit().constData(),
             boost::program_options::value<T>(&location)->default_value(location),
             desciption.toLocal8Bit().constData());
    m_printable.push_back(new Variable<T>(name,location));
}

template <> inline void Parser::createVariable<QString>(const QString& name, QString &location, const QString desciption)
{
    m_description.add_options()
            (name.toLocal8Bit().constData(),
             boost::program_options::value<QString>(&location),
             desciption.toLocal8Bit().constData());
    m_printable.push_back(new Variable<QString>(name,location));
}

}

inline std::istream& operator>>(std::istream& stream, QString& string)
{
    std::string result;
    stream >> result;
    string = QString::fromStdString(result);
    return stream;
}
#endif // PARSER_H
