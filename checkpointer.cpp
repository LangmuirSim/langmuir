#include "checkpointer.h"
#include "chargeagent.h"
#include "output.h"
#include "world.h"

#include <QFile>

namespace Langmuir
{

Checkpointer::Checkpointer(World &world, QObject *parent) :
    QObject(parent), m_world(world)
{
}

void Checkpointer::load(const QString& fileName)
{
}

void Checkpointer::save(const QString& fileName)
{
    OutputInfo info(fileName,&m_world.parameters());

    QString bak1 = info.absoluteFilePath() + ".bak";
    QString bak2 = info.absoluteFilePath() + ".bak.bak";

    if ( info.exists() )
    {
        if ( QFile::exists(bak1) )
        {
            if ( QFile::exists(bak2) )
            {
                // out.bin + out.bin.1 + out.bin.2
                qFatal("can not create checkpoint safely");
            }
            // out.bin + out.bin.1
            QFile::rename(bak1,bak2);
        }
        // out.bin
        QFile::rename(info.absoluteFilePath(),bak1);
    }

    QFile handle(info.absoluteFilePath());
    handle.open(QIODevice::WriteOnly);
    QDataStream stream(&handle);
    stream.setVersion(QDataStream::Qt_4_7);
    stream.setFloatingPointPrecision(QDataStream::DoublePrecision);

    // Magic Number
    stream << (quint32)0xA0B0C0D0;

    // Version
    stream << (qint32 )100;

    // Electrons
    stream << qint32(m_world.numElectronAgents());
    for (int i = 0; i < m_world.numElectronAgents(); i++)
    {
        stream << qint32(m_world.electrons().at(i)->getCurrentSite());
    }

    // Holes
    stream << qint32(m_world.numHoleAgents());
    for (int i = 0; i < m_world.numHoleAgents(); i++)
    {
        stream << qint32(m_world.holes().at(i)->getCurrentSite());
    }

    // Defects
    stream << qint32(m_world.numDefects());
    for (int i = 0; i < m_world.numDefects(); i++)
    {
        stream << qint32(m_world.defectSiteIDs().at(i));
    }

    // Traps
    stream << qint32(m_world.numTraps());
    for (int i = 0; i < m_world.numTraps(); i++)
    {
        stream << qint32(m_world.trapSiteIDs().at(i));
    }
    for (int i = 0; i < m_world.numTraps(); i++)
    {
        stream << m_world.trapSitePotentials().at(i);
    }

    // Random Seed
    //stream << quint64(parameters().randomSeed);

    // Current Step
    //stream << quint32(parameters().currentStep);

    // Close File
    handle.close();

    // Remove the oldest backup file
    if ( QFile::exists(bak2) ) { QFile::remove(bak2); }
}

void Checkpointer::check(QDataStream& stream, const QString& message)
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
