#ifndef KEYVALUEPARSER_H
#define KEYVALUEPARSER_H

#include <QObject>

#include "variable.h"
#include "parameters.h"
#include "world.h"

namespace Langmuir
{

//! Read the parameters and store them in the correct place
class KeyValueParser : public QObject
{
    Q_OBJECT
public:
    KeyValueParser(World& world, QObject *parent = 0);
   ~KeyValueParser();

    SimulationParameters& parameters();
    void parse(const QString& line);
    friend std::ostream& operator<<(std::ostream &stream, const KeyValueParser &keyValueParser);

private:
    QMap<QString,Variable*> m_variableMap;
    SimulationParameters m_parameters;
    World& m_world;

    template <typename T> void registerVariable(
        const QString &key,
        T &value,
        Variable::VariableMode mode = 0);
};

template <typename T> void KeyValueParser::registerVariable(
    const QString &key,
    T &value,
    Variable::VariableMode mode)
{
    if (m_variableMap.find(key) != m_variableMap.end())
    {
        qFatal("variable already registered; key %s",
               qPrintable(key));
    }
    m_variableMap.insert(key, new TypedVariable<T>(key,value,mode,this));
}

}

#endif // KEYVALUEPARSER_H
