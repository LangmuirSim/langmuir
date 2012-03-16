#ifndef READER_H
#define READER_H

#include <QObject>

#include "variable.h"
#include "parameters.h"
#include "world.h"

namespace Langmuir
{

//! Read the parameters and store them in the correct place
class Reader : public QObject
{
    Q_OBJECT
public:
    explicit Reader(World& world, QObject *parent = 0);
    ~Reader();

    void load(const QString& fileName);
    void save(const QString& fileName = "%path/%stub.inp");

    friend QTextStream& operator<<(QTextStream& stream, const Reader &reader);
    friend QDataStream& operator<<(QDataStream &stream, const Reader &reader);
    friend QDataStream& operator>>(QDataStream &stream, const Reader &reader);

    SimulationParameters& parameters();

private:
    QMap<QString,Variable*> m_variableMap;
    SimulationParameters m_parameters;
    World& m_world;

    void parse(const QString& line);
    template <typename T> void registerVariable(
        const QString &key,
        T &value,
        Variable::VariableMode mode = 0);
};

template <typename T> void Reader::registerVariable(
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

#endif // READER_H
