#include "checkpointer.h"
#include "chargeagent.h"
#include "output.h"
#include "world.h"
#include "rand.h"

#include <QFile>
#include <fstream>

namespace Langmuir
{

CheckPointer::CheckPointer(World &world, QObject *parent) :
    QObject(parent), m_world(world), m_magic(0xA0B0C0D0), m_version(107)
{
}

void CheckPointer::load(const QString &fileName, SimulationSiteInfo &siteInfo)
{
    // Clear the site info
    siteInfo.clear();

    // Create binary file
    QFile handle(fileName);
    if (!handle.open(QIODevice::ReadOnly))
    {
        qFatal("can not open checkpoint file: %s",
               qPrintable(fileName));
    }
    QDataStream stream(&handle);
    stream.setVersion(QDataStream::Qt_4_7);
    stream.setFloatingPointPrecision(QDataStream::DoublePrecision);

    // Magic Number
    quint32 magic;
    stream >> magic;
    checkDataStream(stream,QString("expected magic number"));
    if (magic != m_magic)
    {
        qFatal("can not understand binary file format.\n"
               "the magic number does not match: %d != %d", magic, m_magic);
    }

    // Version
    qint32 version;
    stream >> version;
    checkDataStream(stream,QString("expected version number"));
    if (version < 100)
    {
        qFatal("can not understand binary file format.\n"
               "the version number is too low: %d < %d", version, m_version);
    }

    // Electrons
    if (version > 100)
    {
        qint32 num_electrons = 0;
        stream >> num_electrons;
        checkDataStream(stream,"expected number of electrons");
        for (int i = 0; i < num_electrons; i++)
        {
            qint32 value;
            stream >> value;
            checkDataStream(stream,QString("expected electron %1 site id").arg(i));
            siteInfo.electrons.push_back(value);
        }
    }

    // Holes
    if (version > 101)
    {
        qint32 num_holes = 0;
        stream >> num_holes;
        checkDataStream(stream,"expected number of holes");
        for (int i = 0; i < num_holes; i++)
        {
            qint32 value;
            stream >> value;
            checkDataStream(stream,QString("expected hole %1 site id").arg(i));
            siteInfo.holes.push_back(value);
        }
    }

    // Defects
    if (version > 102)
    {
        qint32 num_defects = 0;
        stream >> num_defects;
        checkDataStream(stream,"expected number of defects");
        for (int i = 0; i < num_defects; i++)
        {
            qint32 value;
            stream >> value;
            checkDataStream(stream,QString("expected defect %1 site id").arg(i));
            siteInfo.defects.push_back(value);
        }
    }

    // Traps
    if (version > 103)
    {
        qint32 num_traps = 0;
        stream >> num_traps;
        checkDataStream(stream,"expected number of traps");
        for (int i = 0; i < num_traps; i++)
        {
            qint32 value;
            stream >> value;
            checkDataStream(stream,QString("expected trap %1 site id").arg(i));
            siteInfo.traps.push_back(value);
        }
        for (int i = 0; i < num_traps; i++)
        {
            qreal value;
            stream >> value;
            checkDataStream(stream,QString("expected trap %1 potential id").arg(i));
            siteInfo.potentials.push_back(value);
        }
    }

    // Load the Random Number Generator State
    if (m_version > 104)
    {
        stream >> m_world.randomNumberGenerator();
        checkDataStream(stream,QString("expected random number generator state"));
        m_world.parameters().randomSeed = m_world.randomNumberGenerator().seed();
    }

    if (version > 105)
    {
        quint32 value;
        stream >> value;
        checkDataStream(stream,QString("expected current.step"));
        m_world.parameters().currentStep = value;
    }

    // Close File
    handle.close();
}

void CheckPointer::save(const QString& fileName)
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
    stream << quint32(m_magic);

    // Version
    stream << qint32(m_version);

    // Electrons
    if (m_version > 100)
    {
        stream << qint32(m_world.numElectronAgents());
        for (int i = 0; i < m_world.numElectronAgents(); i++)
        {
            stream << qint32(m_world.electrons().at(i)->getCurrentSite());
        }
    }

    // Holes
    if (m_version > 101)
    {
    stream << qint32(m_world.numHoleAgents());
    for (int i = 0; i < m_world.numHoleAgents(); i++)
    {
        stream << qint32(m_world.holes().at(i)->getCurrentSite());
    }
    }

    // Defects
    if (m_version > 102)
    {
    stream << qint32(m_world.numDefects());
    for (int i = 0; i < m_world.numDefects(); i++)
    {
        stream << qint32(m_world.defectSiteIDs().at(i));
    }
    }

    // Traps
    if (m_version > 103)
    {
    stream << qint32(m_world.numTraps());
    for (int i = 0; i < m_world.numTraps(); i++)
    {
        stream << qint32(m_world.trapSiteIDs().at(i));
    }
    for (int i = 0; i < m_world.numTraps(); i++)
    {
        stream << qreal(m_world.trapSitePotentials().at(i));
    }
    }

    // Save Random Number Generator State
    if (m_version > 104)
    {
        stream << m_world.randomNumberGenerator();
    }

    // Current Step
    if (m_version > 105)
    {
        stream << quint32(m_world.parameters().currentStep);
    }

    // Close File
    handle.close();

    // Remove the oldest backup file
    if ( QFile::exists(bak2) ) { QFile::remove(bak2); }
}

void CheckPointer::checkDataStream(QDataStream& stream, const QString& message)
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
    default:
    {
        QString error = "binary stream error: QDataStream::Status unknown";
        if (!message.isEmpty()) { error += QString("\n\t%1").arg(message); }
        qFatal("%s",qPrintable(error));
        break;
    }
    }
}

}
