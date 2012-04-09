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
#include "keyvalueparser.h"
#include "checkpointer.h"

namespace Langmuir {

World::World(const QString &fileName, QObject *parent)
    : QObject(parent)
{
    initialize(fileName);
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
    delete m_keyValueParser;
    delete m_checkPointer;
}

CheckPointer& World::checkPointer()
{
    return *m_checkPointer;
}

KeyValueParser& World::keyValueParser()
{
    return *m_keyValueParser;
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

boost::multi_array<double,3>& World::couplingConstants()
{
    return m_couplingConstants;
}

double World::coupling(int dx, int dy, int dz)
{
    return m_couplingConstants[dx][dy][dz];
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

void World::initialize(const QString &fileName)
{
    // Pointers are EVIL
    World &refWorld = *this;

    // Set the Key Value Parser
    m_keyValueParser = new KeyValueParser(refWorld,this);

    // Set the address of the Parameters
    m_parameters = &m_keyValueParser->parameters();

    // Create Random Number Generator using current time
    // Note that checkPointer may either reseed the generator
    // or load a previous RandomState
    m_rand = new Random(0,this);

    // Create CheckPointer
    m_checkPointer = new CheckPointer(refWorld,this);

    // Create Site info
    SimulationSiteInfo siteInfo;

    // Parse the input file
    m_checkPointer->load(fileName,siteInfo);

    // Save the seed that has been used
    m_parameters->randomSeed = m_rand->seed();

    // Create Electron Grid
    m_electronGrid = new Grid(refWorld, this);

    // Create Hole Grid
    m_holeGrid = new Grid(refWorld, this);

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
    placeDefects(siteInfo.defects);

    // Place Electrons
    placeElectrons(siteInfo.electrons);

    // Place Holes
    placeHoles(siteInfo.holes);

    // Zero potential
    potential().setPotentialZero();

    // Set Linear Potential
    potential().setPotentialLinear();

    // Place Traps
    potential().setPotentialTraps(siteInfo.traps,siteInfo.trapPotentials);

    // precalculate and store coulomb interaction energies
    potential().updateInteractionEnergies();

    // precalculate and store coupling constants
    potential().updateCouplingConstants();

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
    if (parameters().seedCharges != 0)
    {
        int tries = 0;
        int maxTries = 10*(electronGrid().volume());
        for(int i = numElectronAgents();
            i < maxElectronAgents() * parameters().seedCharges;)
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
    if (parameters().seedCharges != 0)
    {
        int tries = 0;
        int maxTries = 10*(holeGrid().volume());
        for(int i = numHoleAgents();
            i < maxHoleAgents() * parameters().seedCharges;)
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
