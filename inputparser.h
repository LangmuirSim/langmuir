#ifndef INPUTPARSER_H
#define INPUTPARSER_H

#include <QTextStream>
#include <QObject>
#include <QMap>
#include <QDebug>

#include "parameters.h"

namespace Langmuir
{

class InputParser : public QObject
{
    Q_OBJECT
public:
    explicit InputParser(QObject *parent = 0);
    ~InputParser();
    SimulationParameters& getParameters(int step = 0);
    void parseKeyValue(QString fileName);
    void parserXML(QString fileName);
    int steps();

    void saveParametersXML(int step);
    void saveParameters(int step);

private:

    class AbstractVariable
    {
    public:
        AbstractVariable(QString key, bool write = true)
            : m_key(key), m_set(false), m_write(write) {}
        virtual void setValue(QString value) = 0;
        const bool& wasSet() const { return m_set; }
        const bool& isWritable() const { return m_write; }
        const QString& key() const { return m_key; }
        virtual QString value(int precision) const = 0;
    protected:
        QString m_key;
        bool m_set;
        bool m_write;
        static void convert( QString string, unsigned int &result );
        static void convert( QString string, QString &result );
        static void convert( QString string, double &result );
        static void convert( QString string, bool &result );
        static void convert( QString string, int &result );    
    };

    template <typename T>
    class Variable : public AbstractVariable
    {
    public:
        Variable(QString key, T& value, bool write = true)
            : AbstractVariable(key,write), m_value(value) {}
        void setValue(QString value);
        QString value(int precision) const;
    private:
        T &m_value;
    };

    QList<SimulationParameters> m_parameters;
    QList<QMap<QString,AbstractVariable*> > m_variables;

    template <typename T> void createVariable(QString key, T &value, int step, bool write = true);
    void setValue(QString key, QString value, int step);
    void createSteps(int toStep);
    void setMap(int step);
    void checkStep(int step);
};

template <typename T> void InputParser::createVariable(QString key, T &value, int step, bool write)
{
    if (step < 0 || step > m_parameters.size() || step > m_variables.size())
    {
        qFatal("createVariable requested step(%d) >= steps(%d) defined",steps(),step);
    }
    if (m_variables[step].contains(key))
    {
        qFatal("createVariable requested key(%s) that already existed",qPrintable(key));
    }
    m_variables[step][key] = new Variable<T>(key,value,write);
}

template <typename T> inline QString InputParser::Variable<T>::value(int precision) const
{
    QTextStream stream;
    QString result;
    stream.setString(&result);
    stream.setRealNumberPrecision(precision);
    stream << m_value;
    return result;
}

template <> inline QString InputParser::Variable<bool>::value(int precision) const
{
    QTextStream stream;
    QString result;
    stream.setString(&result);
    stream.setRealNumberPrecision(precision);
    if (m_value == true) stream << "true";
    else stream << "false";
    return result;
}

template <typename T> inline void InputParser::Variable<T>::setValue(QString value)
{
    if (!m_write)
    {
        qFatal("setValue requested parameter %s, which is not writable",
               qPrintable(m_key));
        return;
    }
    convert(value,m_value);
    m_set = true;
}

}
#endif // INPUTPARSER_H
