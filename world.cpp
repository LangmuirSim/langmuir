#include "parameters.h"
#include "openclhelper.h"
#include "chargeagent.h"
#include "sourceagent.h"
#include "drainagent.h"
#include "potential.h"
#include "cubicgrid.h"
#include "writer.h"
#include "world.h"
#include "rand.h"
#include "fluxagent.h"
#include "output.h"

namespace Langmuir {

World::World(SimulationParameters &par, QObject *parent)
    : QObject(parent)
{
    // Set the address of the Parameters passed
    m_parameters = &par;

    // Initialize World
    initialize();
}

World::~World()
{
    for(int i = 0; i < m_sources.size(); i++)
    {
        delete m_sources[i];
    }
    m_sources.clear();

    for(int i = 0; i < m_drains.size(); i++)
    {
        delete m_drains[i];
    }
    m_drains.clear();

    for(int i = 0; i < m_electrons.size(); i++)
    {
        delete m_electrons[i];
    }
    m_electrons.clear();

    for(int i = 0; i < m_holes.size(); i++)
    {
        delete m_holes[i];
    }
    m_holes.clear();

    delete m_rand;
    delete m_potential;
    delete m_electronGrid;
    delete m_holeGrid;
    delete m_logger;
    delete m_ocl;
}

Grid& World::electronGrid()
{
    return *m_electronGrid;
}

Grid& World::holeGrid()
{
    return *m_holeGrid;
}

Potential& World::potential()
{
    return *m_potential;
}

SimulationParameters& World::parameters()
{
    return *m_parameters;
}

Random& World::randomNumberGenerator()
{
    return *m_rand;
}

Logger& World::logger()
{
    return *m_logger;
}

OpenClHelper& World::opencl()
{
    return *m_ocl;
}

QList<SourceAgent*>& World::sources()
{
    return m_sources;
}

QList<DrainAgent*>& World::drains()
{
    return m_drains;
}

QList<FluxAgent*>& World::fluxes()
{
    return m_fluxAgents;
}

QList<ChargeAgent*>& World::electrons()
{
    return m_electrons;
}

QList<ChargeAgent*>& World::holes()
{
    return m_holes;
}

QList<int>& World::defectSiteIDs()
{
    return m_defectSiteIDs;
}

QList<int>& World::trapSiteIDs()
{
    return m_trapSiteIDs;
}

QList<double>& World::trapSitePotentials()
{
    return m_trapSitePotentials;
}

boost::multi_array<double,3>& World::interactionEnergies()
{
    return m_interactionEnergies;
}

int World::maxElectronAgents()
{
    return m_maxElectrons;
}

int World::maxHoleAgents()
{
    return m_maxHoles;
}

int World::maxDefects()
{
    return m_maxDefects;
}

int World::maxTraps()
{
    return m_maxTraps;
}

int World::maxChargeAgents()
{
    return maxElectronAgents() + maxHoleAgents();
}

int World::maxCharges()
{
    return maxChargeAgents() + numDefects();
}

int World::numElectronAgents()
{
    return m_electrons.size();
}

int World::numHoleAgents()
{
    return m_holes.size();
}

int World::numChargeAgents()
{
    return numElectronAgents() + numHoleAgents();
}

int World::numCharges()
{
    return numChargeAgents() + numDefects();
}

int World::numDefects()
{
    return m_defectSiteIDs.size();
}

int World::numTraps()
{
    return m_trapSiteIDs.size();
}

double World::reachedChargeAgents()
{
    if(maxChargeAgents()> 0)
    {
        return double(numChargeAgents())/double(maxChargeAgents())*100.0;
    }
    else
    {
        return 0.0;
    }
}

double World::reachedHoleAgents()
{
    if(maxHoleAgents()> 0)
    {
        return double(numHoleAgents())/double(maxHoleAgents())*100.0;
    }
    else
    {
        return 0.0;
    }
}

double World::reachedElectronAgents()
{
    if(maxElectronAgents()> 0)
    {
        return double(numElectronAgents())/double(maxElectronAgents())*100.0;
    }
    else
    {
        return 0.0;
    }
}

double World::percentHoleAgents()
{
    return double(numHoleAgents())/double(m_holeGrid->volume())*100.0;
}

double World::percentElectronAgents()
{
    return double(numElectronAgents())/double(m_electronGrid->volume())*100.0;
}

void World::saveCheckpointFile(const QString &name)
{
    OutputInfo info(name,m_parameters);

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

    // Create binary file
    QFile handle(info.absoluteFilePath());
    if (!handle.open(QIODevice::WriteOnly))
    {
        qFatal("can not open checkpoint file: %s",qPrintable(name));
    }
    QDataStream stream(&handle);
    stream.setVersion(QDataStream::Qt_4_7);
    stream.setFloatingPointPrecision(QDataStream::DoublePrecision);

    // Electrons
    stream << qint32(numElectronAgents());
    for (int i = 0; i < numElectronAgents(); i++) { stream << qint32(m_electrons.at(i)->getCurrentSite()); }

    // Holes
    stream << qint32(numHoleAgents());
    for (int i = 0; i < numHoleAgents(); i++) { stream << qint32(m_holes.at(i)->getCurrentSite()); }

    // Defects
    stream << qint32(numDefects());
    for (int i = 0; i < numDefects(); i++) { stream << qint32(m_defectSiteIDs.at(i)); }

    // Traps
    stream << qint32(numTraps());
    for (int i = 0; i < numTraps(); i++) { stream << qint32(m_trapSiteIDs.at(i)); }
    for (int i = 0; i < numTraps(); i++) { stream << m_trapSitePotentials.at(i); }

    // Random Seed
    stream << quint64(parameters().randomSeed);

    // Current Step
    stream << quint32(parameters().currentStep);

    // Close File
    handle.close();

    // Remove the oldest backup file
    if ( QFile::exists(bak2) ) { QFile::remove(bak2); }
}

void World::initialize()
{
    // Pointers are EVIL
    World &refWorld = *this;

    // Create data
    QList<qint32> electrons;
    QList<qint32> holes;
    QList<qint32> defects;
    QList<qint32> traps;
    QList<double> potentials;

    if (!parameters().checkFile.isEmpty())
    {
        // Create binary file
        QFile handle(parameters().checkFile);
        if (!handle.open(QIODevice::ReadOnly))
        {
            qFatal("can not open checkpoint file: %s",qPrintable(parameters().checkFile));
        }
        QDataStream stream(&handle);
        stream.setVersion(QDataStream::Qt_4_7);
        stream.setFloatingPointPrecision(QDataStream::DoublePrecision);

        // Electrons
        {
            qint32 num_electrons = 0;
            stream >> num_electrons;
            checkDataStream(stream,"expected number of electrons");
            for (int i = 0; i < num_electrons; i++)
            {
                qint32 value;
                stream >> value;
                checkDataStream(stream,QString("expected electron %1 site id").arg(i));
                electrons.push_back(value);
            }
        }

        // Holes
        {
            qint32 num_holes = 0;
            stream >> num_holes;
            checkDataStream(stream,"expected number of holes");
            for (int i = 0; i < num_holes; i++)
            {
                qint32 value;
                stream >> value;
                checkDataStream(stream,QString("expected hole %1 site id").arg(i));
                holes.push_back(value);
            }
        }

        // Defects
        {
            qint32 num_defects = 0;
            stream >> num_defects;
            checkDataStream(stream,"expected number of defects");
            for (int i = 0; i < num_defects; i++)
            {
                qint32 value;
                stream >> value;
                checkDataStream(stream,QString("expected defect %1 site id").arg(i));
                defects.push_back(value);
            }
        }

        // Traps
        {
            qint32 num_traps = 0;
            stream >> num_traps;
            checkDataStream(stream,"expected number of traps");
            for (int i = 0; i < num_traps; i++)
            {
                qint32 value;
                stream >> value;
                checkDataStream(stream,QString("expected trap %1 site id").arg(i));
                traps.push_back(value);
            }
            for (int i = 0; i < num_traps; i++)
            {
                double value;
                stream >> value;
                checkDataStream(stream,QString("expected trap %1 potential id").arg(i));
                potentials.push_back(value);
            }
        }

        // Random Seed
        stream >> m_parameters->randomSeed;
        checkDataStream(stream,"expected parameter random.seed");

        // Random Seed
        stream >> m_parameters->currentStep;
        checkDataStream(stream,"expected parameter current.step");

        // Close file
        handle.close();

        // Set append parameter so that data is not lost
        m_parameters->outputAppend = true;
    }

    // Create Random Number Generator
    m_rand = new Random(parameters().randomSeed);
    m_parameters->randomSeed = m_rand->seed();

    // Create Electron Grid
    m_electronGrid = new Grid(parameters().gridX, parameters().gridY, parameters().gridZ, this);

    // Create Hole Grid
    m_holeGrid = new Grid(parameters().gridX, parameters().gridY, parameters().gridZ, this);

    // Calculate the max number of holes
    m_maxHoles = parameters().holePercentage*double(holeGrid().volume());

    // Calculate the max number of electrons
    m_maxElectrons = parameters().electronPercentage*double(electronGrid().volume());

    // Calculate the max number of electrons
    m_maxDefects = parameters().defectPercentage*double(electronGrid().volume());

    // Calculate the max number of traps
    m_maxTraps = parameters().trapPercentage*double(electronGrid().volume());

    // Create Potential Calculator
    m_potential = new Potential(refWorld, this);

    // Create OpenCL Objects
    m_ocl = new OpenClHelper(refWorld, this);

    if ( parameters().simulationType == "transistor" )
    {
        // Create Transistor Sources
        m_sources.push_back(new ElectronSourceAgent(refWorld, Grid::Left, this));
        m_sources.last()->setRate(parameters().sourceRate);

        // Create Transistor Drains
        m_drains.push_back(new ElectronDrainAgent(refWorld, Grid::Right, this));
        m_drains.last()->setRate(parameters().drainRate);
    }
    else if ( parameters().simulationType == "solarcell" )
    {
        // Create Solar Cell Sources
        m_sources.push_back(new ExcitonSourceAgent(refWorld, this));
        m_sources.last()->setRate(parameters().sourceRate);

        // Create Solar Cell Drains
        m_drains.push_back(new ElectronDrainAgent(refWorld, Grid::Left, this));
        m_drains.last()->setRate(parameters().drainRate);

        m_drains.push_back(new HoleDrainAgent(refWorld, Grid::Left, this));
        m_drains.last()->setRate(parameters().drainRate);

        m_drains.push_back(new ElectronDrainAgent(refWorld, Grid::Right, this));
        m_drains.last()->setRate(parameters().drainRate);

        m_drains.push_back(new HoleDrainAgent(refWorld, Grid::Right, this));
        m_drains.last()->setRate(parameters().drainRate);
    }
    else
    {
        qFatal("simulation.type(%s) must be transistor or solarcell",qPrintable(parameters().simulationType));
    }

    foreach( FluxAgent *flux, m_sources )
    {
        m_fluxAgents.push_back(flux);
    }

    foreach( FluxAgent *flux, m_drains )
    {
        m_fluxAgents.push_back(flux);
    }

    // Create Logger
    if (parameters().outputIsOn)
    {
        m_logger = new LoggerOn(refWorld, this);
    }
    else
    {
        m_logger = new Logger(refWorld, this);
    }

    // Place Defects
    placeDefects(defects);

    // Place Electrons
    placeElectrons(electrons);

    // Place Holes
    placeHoles(holes);

    // Zero potential
    potential().setPotentialZero();

    // Set Linear Potential
    potential().setPotentialLinear();

    // Place Traps
    potential().setPotentialTraps(traps,potentials);

    // precalculate and store coulomb interaction energies
    potential().updateInteractionEnergies();

    // Generate grid image
    if(parameters().imageDefects)
    {
        logger().saveDefectImage();
    }

    if(parameters().imageTraps)
    {
        logger().saveTrapImage();
    }

    // Output Field Energy
    if(parameters().outputPotential)
    {
        logger().saveGridPotential();
    }

    // Initialize OpenCL
    opencl().initializeOpenCL();
    opencl().toggleOpenCL(parameters().useOpenCL);
}

void World::checkDataStream(QDataStream& stream, const QString& message)
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

void World::placeDefects(const QList<int>& siteIDs)
{
    int count = numDefects();
    int toBePlaced = maxDefects() - count;
    int toBePlacedUsingIDs = siteIDs.size();
    int toBePlacedRandomly = toBePlaced - siteIDs.size();

    if (toBePlacedRandomly < 0)
    {
        qFatal("can not place defects;\n\tmost likely the list "
               "of defect site IDs exceeds the maximum "
               "given by defect.precentage");
    }

    // Place the defects in the siteID list
    for (int i = 0; i < toBePlacedUsingIDs; i++)
    {
        int site = siteIDs.at(i);
        if ( defectSiteIDs().contains(site) )
        {
            qFatal("can not add defect;\n\tdefect already exists");
        }
        electronGrid().registerDefect(site);
        holeGrid().registerDefect(site);
        defectSiteIDs().push_back(site);
        count++;
    }

    // Place the rest of the defects randomly
    int tries = 0;
    int maxTries = 10*(electronGrid().volume());
    for(int i = count; i < maxDefects();)
    {
        tries++;
        int site = randomNumberGenerator()
                .integer(0,electronGrid().volume()-1);

        if (!defectSiteIDs().contains(site))
        {
            electronGrid().registerDefect(site);
            holeGrid().registerDefect(site);
            defectSiteIDs().push_back(site);
            count++;
            i++;
        }

        if (tries > maxTries)
        {
            qFatal("can not seed defects;\n\texceeded max tries(%d)",
                   maxTries);
        }
    }
}

void World::placeElectrons(const QList<int>& siteIDs)
{
    // Create a Source Agent for placing electrons
    ElectronSourceAgent source(*this,Grid::NoFace);
    source.setRate(1.0);

    // Place the electrons in the site ID list
    for (int i = 0; i < siteIDs.size(); i++)
    {
        if (!source.tryToSeed(siteIDs[i]))
        {
            qFatal("can not inject electron at site %d",
                   siteIDs[i]);
        }
    }

    // Place the rest of the electrons randomly if charge seeding is on
    if (parameters().seedCharges)
    {
        int tries = 0;
        int maxTries = 10*(electronGrid().volume());
        for(int i = numElectronAgents();
            i < maxElectronAgents();)
        {
            if(source.tryToSeed()) { ++i; }
            tries++;
            if (tries > maxTries)
            {
                qFatal("can not seed electrons;\n\texceeded max tries(%d)",
                       maxTries);
            }
        }
    }
}

void World::placeHoles(const QList<int>& siteIDs)
{
    // Create a Source Agent for placing holes
    HoleSourceAgent source(*this,Grid::NoFace);
    source.setRate(1.0);

    // Place the holes in the site ID list
    for (int i = 0; i < siteIDs.size(); i++)
    {
        if (!source.tryToSeed(siteIDs[i]))
        {
            qFatal("can not inject hole at site %d",
                   siteIDs[i]);
        }
    }

    // Place the rest of the holes randomly if charge seeding is on
    if (parameters().seedCharges)
    {
        int tries = 0;
        int maxTries = 10*(holeGrid().volume());
        for(int i = numHoleAgents();
            i < maxHoleAgents();)
        {
            if(source.tryToSeed()) { ++i; }
            tries++;
            if (tries > maxTries)
            {
                qFatal("can not seed holes;\n\texceeded max tries(%d)",
                       maxTries);
            }
        }
    }
}

}
