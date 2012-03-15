#ifndef CHECKPOINTER_H
#define CHECKPOINTER_H

#include <QObject>
#include <QDataStream>

#include "parameters.h"

namespace Langmuir
{

class World;

class Checkpointer : public QObject
{
    Q_OBJECT
public:
    explicit Checkpointer(World& world, QObject *parent = 0);

    void load(const QString& fileName, SimulationSiteInfo &siteInfo);
    void save(const QString& fileName);
    void checkDataStream(QDataStream& stream, const QString& message = "");

private:
    World &m_world;
    quint32 m_magic;
    qint32 m_version;
};

}
#endif // CHECKPOINTER_H
