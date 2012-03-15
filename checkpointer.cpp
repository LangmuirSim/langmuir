#include "checkpointer.h"
#include "world.h"

namespace Langmuir
{

Checkpointer::Checkpointer(World &world, QObject *parent) :
    QObject(parent), m_world(world)
{
}

void load(const QString& fileName)
{
}

void save(const QString& fileName)
{
}

void check(QDataStream& stream)
{
}

}
