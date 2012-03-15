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

void check(QDataStream& stream, const QString& message)
{
    switch (stream.status())
    {
    case QDataStream::Ok:
    {
        return;
        break;
    }
    case QDataStream::ReadPastEnd:
    {
        QString error = "binary stream read error: QDataStream::ReadPastEnd";
        if (!message.isEmpty()) { error += QString("\n\t%1").arg(message); }
        qFatal("%s",qPrintable(error));
        break;
    }
    case QDataStream::ReadCorruptData:
    {
        QString error = "binary stream read error: QDataStream::ReadCorruptData";
        if (!message.isEmpty()) { error += QString("\n\t%1").arg(message); }
        qFatal("%s",qPrintable(error));
        break;
    }
    }
}

}
