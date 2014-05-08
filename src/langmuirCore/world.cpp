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
#include "fluxagent.h"
#include "nodefileparser.h"

namespace Langmuir {

World::World(const QString &fileName, int cores, int gpuID, QObject *parent)
    : QObject(parent),
      m_keyValueParser(NULL),
      m_checkPointer(NULL),
      m_electronSourceAgentRight(NULL),
      m_electronSourceAgentLeft(NULL),
      m_holeSourceAgentRight(NULL),
      m_holeSourceAgentLeft(NULL),
      m_excitonSourceAgent(NULL),
      m_electronDrainAgentRight(NULL),
      m_electronDrainAgentLeft(NULL),
      m_holeDrainAgentRight(NULL),
      m_holeDrainAgentLeft(NULL),
      m_recombinationAgent(NULL),
      m_electronGrid(NULL),
      m_holeGrid(NULL),
      m_rand(NULL),
      m_potential(NULL),
      m_parameters(NULL),
      m_logger(NULL),
      m_ocl(NULL),
      m_maxElectrons(0),
      m_maxHoles(0),
      m_maxDefects(0),
      m_maxTraps(0)
{
    initialize(fileName, NULL, NULL, cores, gpuID);
}

World::World(SimulationParameters &parameters, int cores, int gpuID, QObject *parent)
    : QObject(parent),
      m_keyValueParser(NULL),
      m_checkPointer(NULL),
      m_electronSourceAgentRight(NULL),
      m_electronSourceAgentLeft(NULL),
      m_holeSourceAgentRight(NULL),
      m_holeSourceAgentLeft(NULL),
      m_excitonSourceAgent(NULL),
      m_electronDrainAgentRight(NULL),
      m_electronDrainAgentLeft(NULL),
      m_holeDrainAgentRight(NULL),
      m_holeDrainAgentLeft(NULL),
      m_recombinationAgent(NULL),
      m_electronGrid(NULL),
      m_holeGrid(NULL),
      m_rand(NULL),
      m_potential(NULL),
      m_parameters(NULL),
      m_logger(NULL),
      m_ocl(NULL),
      m_maxElectrons(0),
      m_maxHoles(0),
      m_maxDefects(0),
      m_maxTraps(0)
{
    initialize("", &parameters, NULL, cores, gpuID);
}

World::World(SimulationParameters &parameters, ConfigurationInfo &configInfo, int cores, int gpuID, QObject *parent)
    : QObject(parent),
      m_keyValueParser(NULL),
      m_checkPointer(NULL),
      m_electronSourceAgentRight(NULL),
      m_electronSourceAgentLeft(NULL),
      m_holeSourceAgentRight(NULL),
      m_holeSourceAgentLeft(NULL),
      m_excitonSourceAgent(NULL),
      m_electronDrainAgentRight(NULL),
      m_electronDrainAgentLeft(NULL),
      m_holeDrainAgentRight(NULL),
      m_holeDrainAgentLeft(NULL),
      m_recombinationAgent(NULL),
      m_electronGrid(NULL),
      m_holeGrid(NULL),
      m_rand(NULL),
      m_potential(NULL),
      m_parameters(NULL),
      m_logger(NULL),
      m_ocl(NULL),
      m_maxElectrons(0),
      m_maxHoles(0),
      m_maxDefects(0),
      m_maxTraps(0)
{
    initialize("", &parameters, &configInfo, cores, gpuID);
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

QList<SourceAgent*>& World::eSources()
{
    return m_eSources;
}

QList<SourceAgent*>& World::hSources()
{
    return m_hSources;
}

QList<SourceAgent*>& World::xSources()
{
    return m_xSources;
}

QList<DrainAgent*>& World::drains()
{
    return m_drains;
}

QList<DrainAgent*>& World::eDrains()
{
    return m_eDrains;
}

QList<DrainAgent*>& World::hDrains()
{
    return m_hDrains;
}

QList<DrainAgent*>& World::xDrains()
{
    return m_xDrains;
}

QList<FluxAgent*>& World::fluxes()
{
    return m_fluxAgents;
}

ElectronSourceAgent& World::electronSourceAgentRight()
{
    return *m_electronSourceAgentRight;
}

ElectronSourceAgent& World::electronSourceAgentLeft()
{
    return *m_electronSourceAgentLeft;
}

HoleSourceAgent& World::holeSourceAgentRight()
{
    return *m_holeSourceAgentRight;
}

HoleSourceAgent& World::holeSourceAgentLeft()
{
    return *m_holeSourceAgentLeft;
}

ExcitonSourceAgent& World::excitonSourceAgent()
{
    return *m_excitonSourceAgent;
}

ElectronDrainAgent& World::electronDrainAgentRight()
{
    return *m_electronDrainAgentRight;
}

ElectronDrainAgent& World::electronDrainAgentLeft()
{
    return *m_electronDrainAgentLeft;
}

HoleDrainAgent& World::holeDrainAgentRight()
{
    return *m_holeDrainAgentRight;
}

HoleDrainAgent& World::holeDrainAgentLeft()
{
    return *m_holeDrainAgentLeft;
}

RecombinationAgent& World::recombinationAgent()
{
    return *m_recombinationAgent;
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

boost::multi_array<double,3>& World::R1()
{
    return m_R1;
}

boost::multi_array<double,3>& World::R2()
{
    return m_R2;
}

boost::multi_array<double,3>& World::iR()
{
    return m_iR;
}

boost::multi_array<double,3>& World::eR()
{
    return m_eR;
}

boost::multi_array<double, 3>& World::sI()
{
    return m_sI;
}

boost::multi_array<double,3>& World::couplingConstants()
{
    return m_couplingConstants;
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

int World::maxChargeAgentsAndChargedDefects()
{
    if (parameters().defectsCharge)
    {
        return maxChargeAgents() + numDefects();
    }
    return maxChargeAgents();
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

int World::electronsMinusHoles()
{
    return numElectronAgents() - numHoleAgents();
}

int World::holesMinusElectrons()
{
    return numHoleAgents() - numElectronAgents();
}

bool World::chargesAreBalanced()
{
    return numHoleAgents() == numElectronAgents();
}

int World::numChargeAgentsAndChargedDefects()
{
    if (parameters().defectsCharge)
    {
        return numChargeAgents() + numDefects();
    }
    return numChargeAgents();
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

bool World::atMaxElectrons()
{
    return numElectronAgents() >= maxElectronAgents();
}

bool World::atMaxHoles()
{
    return numHoleAgents() >= maxHoleAgents();
}

bool World::atMaxCharges()
{
    return numChargeAgents() >= maxChargeAgents();
}

void World::alterMaxThreads(int cores)
{
    QThreadPool& threadPool = *QThreadPool::globalInstance();
    int maxThreadCount = threadPool.maxThreadCount();

    if (cores < 0) {
        cores = maxThreadCount;
    }

    if (cores > maxThreadCount) {
        qFatal("langmuir: requested %d cores, max cores=%d", cores, maxThreadCount);
    }

    m_parameters->maxThreads = cores;

    threadPool.setMaxThreadCount(cores);
    qDebug("langmuir: QThreadPool::maxThreadCount set to %d", threadPool.maxThreadCount());
}

void World::initialize(const QString &fileName, SimulationParameters *pparameters, ConfigurationInfo *pconfigInfo, int cores, int gpuID)
{
    // check function arguments
    if (fileName.isEmpty()) {
        if (pparameters == NULL) {
            qFatal("langmuir: no simulation parameters passed to World");
        }
    } else {
        if (pparameters != NULL) {
            qFatal("langmuir: both input file and simulation parameters passed to World");
        }
    }

    // Pointers are EVIL
    World &refWorld = *this;

    // Set the Key Value Parser
    // keyValueParser is an object that maps between parameter
    // names and their locations in the simulationParameters struct
    // It is also the global location of the simulationParameters struct,
    // that a lot of objects carry around references to
    m_keyValueParser = new KeyValueParser(refWorld,this);

    // Copy passed parameters
    if (pparameters != NULL) {
        qDebug("langmuir: copying passed parameters");
        m_keyValueParser->parameters() = *pparameters;
    }

    // Set the address of the Parameters
    m_parameters = &m_keyValueParser->parameters();

    // Create Random Number Generator using current time
    // Note that checkPointer may either reseed the generator
    // or load a previous RandomState
    m_rand = new Random(0,this);

    // Create CheckPointer
    // In retrospect, checkPointer is an amazingly terrible name.
    // It has nothing to do with "pointers".
    // checkPointer handles the checkpoint files (simulation input)
    m_checkPointer = new CheckPointer(refWorld, this);

    // Create Site info
    // This is temporary storage for info (like electron locations)
    // loaded in the simulation input file
    ConfigurationInfo configInfo;

    // Copy passed config info
    if (pconfigInfo != NULL) {
        qDebug("langmuir: copying passed configuration info");
        configInfo = *pconfigInfo;
    }

    // Parse the input file
    if (!fileName.isEmpty()) {
        m_checkPointer->load(fileName, configInfo);
    }
    else {
        qDebug("langmuir: skipping input file");
        qDebug("langmuir: seeding random number generator with random.seed = %d",
               (unsigned int)m_parameters->randomSeed);
        m_rand->seed(m_parameters->randomSeed);
    }
    checkSimulationParameters(*m_parameters);

    // Change the number of threads
    NodeFileParser nfparser;
    QString hostName = nfparser.hostName();

    // Use nodefile if cores wasn't given
    if (cores < 0) {
        cores = nfparser.numProc(hostName);
    }

    // Use gpufile if gpuID wasn't given
    if (gpuID < 0) {
        gpuID = nfparser.GPUid(hostName, 0);
    }

    // Change the number of threads
    alterMaxThreads(cores);

    // Save the seed that has been used
    m_parameters->randomSeed = m_rand->seed();
    qDebug() << "langmuir: random.seed is" << parameters().randomSeed;

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

    // Create SourceAgents
    createSources();

    // Create DrainAgents
    createDrains();

    // set FluxInfo
    setFluxInfo(configInfo.fluxInfo);

    // Create Logger
    m_logger = new Logger(refWorld, this);

    // Place Defects
    placeDefects(configInfo.defects);

    // Place Electrons
    placeElectrons(configInfo.electrons);

    // Place Holes
    placeHoles(configInfo.holes);

    // Zero potential
    potential().setPotentialZero();

    // Set Linear Potential
    potential().setPotentialLinear();

    // Set Gate Potential (does nothing is slope.z is zero or if there is only 1 layer)
    potential().setPotentialGate();

    // Place Traps
    potential().setPotentialTraps(configInfo.traps,configInfo.trapPotentials);

    // precalculate and store coulomb interaction energies
    potential().precalculateArrays();

    // precalculate and store coupling constants
    potential().updateCouplingConstants();

    // Initialize OpenCL
    opencl().initializeOpenCL(gpuID);
    opencl().toggleOpenCL(parameters().useOpenCL);
}

void World::placeDefects(const QList<int>& siteIDs)
{
    qDebug("langmuir: World::placeDefects");

    int num = numDefects();
    int max = maxDefects();
    int toBePlaced = max;
    int toBePlacedIDs = siteIDs.size();
    int toBeSeeded = toBePlaced - siteIDs.size();
    if (toBeSeeded < 0) toBeSeeded = 0;
    if (toBePlacedIDs < 0) toBePlacedIDs = 0;

    qDebug("langmuir: defects allowed = %d", max);

    // Only call this function when there are no defects
    if (num > 0) {
        qDebug("langmuir: defects found = %d", num);
        qFatal("langmuir: can not call World::placeDefects with found > 0");
    }

    if ((toBePlaced < 0) || (toBePlaced > max))
    {
        qDebug("langmuir: defects to place = %d", toBePlaced);
        qFatal("langmuir: invalid number of defects to be placed");
    }

    if ((toBePlacedIDs < 0) || (toBePlacedIDs > max))
    {
        qDebug("langmuir: defects in checkpoint = %d", toBePlacedIDs);
        qFatal("langmuir: invalid number of defects in checkpoint");
    }

    if ((toBeSeeded < 0) || (toBeSeeded > max))
    {
        qDebug("langmuir: defects to seed = %d", toBeSeeded);
        qFatal("langmuir: invalid number of defects to be seeded");
    }

    if (toBePlacedIDs + toBeSeeded != toBePlaced)
    {
        qDebug("langmuir: defects to place = %d", toBePlaced);
        qDebug("langmuir: defects in checkpoint = %d", toBePlacedIDs);
        qDebug("langmuir: defects to seed = %d", toBeSeeded);
        qFatal("langmuir: inconsistant defect input");
    }

    // Place the defects in the siteID list
    if (toBePlaced == 0) {
        qDebug("langmuir: nothing to be done for defects");
        return;
    }

    if (toBePlacedIDs > 0) {
        qDebug("langmuir: placing %d defects from checkpoint", toBePlacedIDs);
        for (int i = 0; i < toBePlacedIDs; i++)
        {
            int site = siteIDs.at(i);
            if (defectSiteIDs().contains(site) )
            {
                qDebug("langmuir: can not add defect");
                qFatal("langmuir: defect already exists");
            }
            electronGrid().registerDefect(site);
            holeGrid().registerDefect(site);
            defectSiteIDs().push_back(site);
            num++;
        }
    }

    // Place the rest of the defects randomly
    if (toBeSeeded > 0)
    {
        int tries = 0;
        int maxTries = 10 * (electronGrid().volume());

        qDebug("langmuir: seeding %d defects", toBeSeeded);
        for(int i = num; i < max;)
        {
            tries++;
            int site = randomNumberGenerator().integer(0, electronGrid().volume() - 1);

            if (!defectSiteIDs().contains(site))
            {
                electronGrid().registerDefect(site);
                holeGrid().registerDefect(site);
                defectSiteIDs().push_back(site);
                num++;
                i++;
            }

            if (tries > maxTries)
            {
                qDebug("langmuir: exceeded max tries (%d)", maxTries);
                qFatal("langmuir: can not seed defects");
            }
        }
    }
    qDebug("langmuir: placed %d defects", numDefects());
}

void World::placeElectrons(const QList<int>& siteIDs)
{
    qDebug("langmuir: World::placeElectrons");

    // Only call this function when there are no electrons
    int num = numElectronAgents();
    if (num != 0) {
        qDebug("langmuir: found %d electrons", num);
        qFatal("langmuir: can not call World::placeElectrons()");
    }

    int max = maxElectronAgents();
    int toBePlacedIDs = siteIDs.size();
    int maxSeeded = max * parameters().seedCharges;
    int toBeSeeded = maxSeeded - toBePlacedIDs;
    if (toBeSeeded < 0) toBeSeeded = 0;
    int toBePlaced = toBePlacedIDs + toBeSeeded;

    qDebug("langmuir: electrons allowed = %d", max);

    // Only call this function when there are no electrons
    if (num > 0) {
        qDebug("langmuir: electrons found = %d", num);
        qFatal("langmuir: can not call World::placeElectrons with found > 0");
    }

    if ((toBePlaced < 0) || (toBePlaced > max))
    {
        qDebug("langmuir: electrons to place = %d", toBePlaced);
        qFatal("langmuir: invalid number of electrons to be placed");
    }

    if ((toBePlacedIDs < 0) || (toBePlacedIDs > max))
    {
        qDebug("langmuir: electrons in checkpoint = %d", toBePlacedIDs);
        qFatal("langmuir: invalid number of electrons in checkpoint");
    }

    if ((toBeSeeded < 0) || (toBeSeeded > max))
    {
        qDebug("langmuir: electrons to seed = %d", toBeSeeded);
        qFatal("langmuir: invalid number of electrons to be seeded");
    }

    if (toBePlacedIDs + toBeSeeded != toBePlaced)
    {
        qDebug("langmuir: electrons to place = %d", toBePlaced);
        qDebug("langmuir: electrons in checkpoint = %d", toBePlacedIDs);
        qDebug("langmuir: electrons to seed = %d", toBeSeeded);
        qFatal("langmuir: inconsistant electrons input");
    }

    // Place the electrons in the siteID list
    if (toBePlaced == 0) {
        qDebug("langmuir: nothing to be done for electrons");
        return;
    }

    // Create a Source Agent for placing electrons
    ElectronSourceAgent source(*this,Grid::NoFace);
    source.setRate(1.0);

    // Place the electrons in the site ID list
    if (toBePlacedIDs > 0)
    {
        qDebug("langmuir: placing %d electrons from checkpoint", toBePlacedIDs);
        for (int i = 0; i < siteIDs.size(); i++)
        {
            if (!source.tryToSeed(siteIDs[i]))
            {
                qFatal("langmuir: can not inject electron at site %d",
                       siteIDs[i]);
            }
        }
    }

    // Place the rest of the electrons randomly if charge seeding is on
    if (toBeSeeded > 0)
    {
        qDebug("langmuir: seeding %d electrons", toBeSeeded);
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
                    qDebug("langmuir: can not seed electrons");
                    qFatal("exceeded max tries(%d)", maxTries);
                }
            }
        }
    }

    qDebug("langmuir: placed %d electrons", numElectronAgents());
}

void World::placeHoles(const QList<int>& siteIDs)
{
    qDebug("langmuir: World::placeHoles");

    // Only call this function when there are no holes
    int num = numHoleAgents();
    if (num != 0) {
        qDebug("langmuir: found %d holes", num);
        qFatal("langmuir: can not call World::placeHoles()");
    }

    int max = maxHoleAgents();
    int toBePlacedIDs = siteIDs.size();
    int maxSeeded = max * parameters().seedCharges;
    int toBeSeeded = maxSeeded - toBePlacedIDs;
    if (toBeSeeded < 0) toBeSeeded = 0;
    int toBePlaced = toBePlacedIDs + toBeSeeded;

    qDebug("langmuir: holes allowed = %d", max);

    // Only call this function when there are no holes
    if (num > 0) {
        qDebug("langmuir: holes found = %d", num);
        qFatal("langmuir: can not call World::placeHoles with found > 0");
    }

    if ((toBePlaced < 0) || (toBePlaced > max))
    {
        qDebug("langmuir: holes to place = %d", toBePlaced);
        qFatal("langmuir: invalid number of holes to be placed");
    }

    if ((toBePlacedIDs < 0) || (toBePlacedIDs > max))
    {
        qDebug("langmuir: holes in checkpoint = %d", toBePlacedIDs);
        qFatal("langmuir: invalid number of holes in checkpoint");
    }

    if ((toBeSeeded < 0) || (toBeSeeded > max))
    {
        qDebug("langmuir: holes to seed = %d", toBeSeeded);
        qFatal("langmuir: invalid number of holes to be seeded");
    }

    if (toBePlacedIDs + toBeSeeded != toBePlaced)
    {
        qDebug("langmuir: holes to place = %d", toBePlaced);
        qDebug("langmuir: holes in checkpoint = %d", toBePlacedIDs);
        qDebug("langmuir: holes to seed = %d", toBeSeeded);
        qFatal("langmuir: inconsistant holes input");
    }

    // Place the holes in the siteID list
    if (toBePlaced == 0) {
        qDebug("langmuir: nothing to be done for holes");
        return;
    }

    // Create a Source Agent for placing holes
    HoleSourceAgent source(*this,Grid::NoFace);
    source.setRate(1.0);

    // Place the holes in the site ID list
    if (toBePlacedIDs > 0)
    {
        qDebug("langmuir: placing %d holes from checkpoint", toBePlacedIDs);
        for (int i = 0; i < siteIDs.size(); i++)
        {
            if (!source.tryToSeed(siteIDs[i]))
            {
                qFatal("langmuir: can not inject hole at site %d",
                       siteIDs[i]);
            }
        }
    }

    // Place the rest of the holes randomly if charge seeding is on
    if (toBeSeeded > 0)
    {
        qDebug("langmuir: seeding %d holes", toBeSeeded);
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
                    qDebug("langmuir: can not seed electrons");
                    qFatal("exceeded max tries(%d)", maxTries);
                }
            }
        }
    }

    qDebug("langmuir: placed %d holes", numHoleAgents());
}

void World::createSources()
{
    qDebug("langmuir: World::createSources");

    World &refWorld = *this;

    // Left electron source
    m_electronSourceAgentLeft = new ElectronSourceAgent(refWorld, Grid::Left, this);
    m_sources.push_back(m_electronSourceAgentLeft);
    m_eSources.push_back(m_electronSourceAgentLeft);
    m_fluxAgents.push_back(m_electronSourceAgentLeft);

    // Right electron source
    m_electronSourceAgentRight = new ElectronSourceAgent(refWorld, Grid::Right, this);
    m_sources.push_back(m_electronSourceAgentRight);
    m_eSources.push_back(m_electronSourceAgentRight);
    m_fluxAgents.push_back(m_electronSourceAgentRight);

    // Left hole source
    m_holeSourceAgentLeft = new HoleSourceAgent(refWorld, Grid::Left, this);
    m_sources.push_back(m_holeSourceAgentLeft);
    m_hSources.push_back(m_holeSourceAgentLeft);
    m_fluxAgents.push_back(m_holeSourceAgentLeft);

    // Right hole source
    m_holeSourceAgentRight = new HoleSourceAgent(refWorld, Grid::Right, this);
    m_sources.push_back(m_holeSourceAgentRight);
    m_hSources.push_back(m_holeSourceAgentRight);
    m_fluxAgents.push_back(m_holeSourceAgentRight);

    // Exciton source
    m_excitonSourceAgent = new ExcitonSourceAgent(refWorld, this);
    m_sources.push_back(m_excitonSourceAgent);
    m_xSources.push_back(m_excitonSourceAgent);
    m_fluxAgents.push_back(m_excitonSourceAgent);

    // Set up rates
    if (parameters().simulationType == "transistor")
    {
        // Turn off excitons
        m_excitonSourceAgent->setRate(0.0);

        // Set rates
        m_electronSourceAgentLeft->setRateSmartly(parameters().eSourceLRate, parameters().sourceRate);
        m_electronSourceAgentRight->setRateSmartly(parameters().eSourceRRate, 0.0);
        m_holeSourceAgentRight->setRateSmartly(parameters().hSourceRRate, 0.0);
        m_holeSourceAgentLeft->setRateSmartly(parameters().hSourceLRate, 0.0);
    }
    else if (parameters().simulationType == "solarcell")
    {
        // Turn on excitons
        m_excitonSourceAgent->setRateSmartly(parameters().generationRate, parameters().sourceRate);

        // Set other rates
        m_electronSourceAgentRight->setRateSmartly(parameters().eSourceRRate, 0.0);
        m_electronSourceAgentLeft->setRateSmartly(parameters().eSourceLRate, 0.0);
        m_holeSourceAgentRight->setRateSmartly(parameters().hSourceRRate, 0.0);
        m_holeSourceAgentLeft->setRateSmartly(parameters().hSourceLRate, 0.0);

        // Scale the generation.rate according to the area of the simulation cell
        if ( parameters().sourceScaleArea > 0 && parameters().sourceScaleArea != m_electronGrid->xyPlaneArea())
        {
            double oldRate = m_excitonSourceAgent->rate();
            double scaleFactor = m_electronGrid->xyPlaneArea() / double(parameters().sourceScaleArea);
            double newRate = oldRate * scaleFactor;
            m_excitonSourceAgent->setRate(newRate);
        }
    }
    else
    {
        qFatal("langmuir: unknown simulation.type - can not create sources");
    }

    qDebug("langmuir: e.source.r.rate = %.3g", m_electronSourceAgentRight->rate());
    qDebug("langmuir: e.source.l.rate = %.3g", m_electronSourceAgentLeft->rate());
    qDebug("langmuir: h.source.r.rate = %.3g", m_holeSourceAgentRight->rate());
    qDebug("langmuir: h.source.l.rate = %.3g", m_holeSourceAgentLeft->rate());
    qDebug("langmuir: generation.rate = %.3g", m_excitonSourceAgent->rate());

    if ((m_electronSourceAgentRight->rate() == 0) &&
        (m_electronSourceAgentLeft->rate() == 0) &&
        (m_holeSourceAgentRight->rate() == 0) &&
        (m_holeSourceAgentLeft->rate() == 0) &&
        (m_excitonSourceAgent->rate() == 0))
    {
        qFatal("langmuir: all sources have zero rate");
    }

    if (maxElectronAgents() == 0)
    {
        if (m_electronSourceAgentRight->rate() > 0)
        {
            qFatal("langmuir: e.source.rate.r = %.3g, yet maxElectronAgents() == %d",
                   m_electronSourceAgentRight->rate(), maxElectronAgents());
        }
        if (m_electronSourceAgentLeft->rate() > 0)
        {
            qFatal("langmuir: e.source.rate.l = %.3g, yet maxElectronAgents() == %d",
                   m_electronSourceAgentLeft->rate(), maxElectronAgents());
        }
        if (m_excitonSourceAgent->rate() > 0)
        {
            qFatal("langmuir: generation.rate = %.3g, yet maxElectronAgents() == %d",
                   m_excitonSourceAgent->rate(), maxElectronAgents());
        }
    }

    if (maxHoleAgents() == 0)
    {
        if (m_holeSourceAgentRight->rate() > 0)
        {
            qFatal("langmuir: h.source.rate.r = %.3g, yet maxHoleAgents() == %d",
                   m_holeSourceAgentRight->rate(), maxHoleAgents());
        }
        if (m_holeSourceAgentLeft->rate() > 0)
        {
            qFatal("langmuir: h.source.rate.l = %.3g, yet maxHoleAgents() == %d",
                   m_holeSourceAgentLeft->rate(), maxHoleAgents());
        }
        if (m_excitonSourceAgent->rate() > 0)
        {
            qFatal("langmuir: generation.rate = %.3g, yet maxHoleAgents() == %d",
                   m_excitonSourceAgent->rate(), maxHoleAgents());
        }
    }
}

void World::createDrains()
{
    qDebug("langmuir: World::createDrains");

    World &refWorld = *this;

    // Left electron drain
    m_electronDrainAgentLeft  = new ElectronDrainAgent(refWorld, Grid::Left, this);
    m_drains.push_back(m_electronDrainAgentLeft);
    m_eDrains.push_back(m_electronDrainAgentLeft);
    m_fluxAgents.push_back(m_electronDrainAgentLeft);

    // Right electron drain
    m_electronDrainAgentRight = new ElectronDrainAgent(refWorld, Grid::Right, this);
    m_drains.push_back(m_electronDrainAgentRight);
    m_eDrains.push_back(m_electronDrainAgentRight);
    m_fluxAgents.push_back(m_electronDrainAgentRight);

    // Left hole drain
    m_holeDrainAgentLeft = new HoleDrainAgent(refWorld, Grid::Left, this);
    m_drains.push_back(m_holeDrainAgentLeft);
    m_hDrains.push_back(m_holeDrainAgentLeft);
    m_fluxAgents.push_back(m_holeDrainAgentLeft);

    // Right hole drain
    m_holeDrainAgentRight = new HoleDrainAgent(refWorld, Grid::Right, this);
    m_drains.push_back(m_holeDrainAgentRight);
    m_hDrains.push_back(m_holeDrainAgentRight);
    m_fluxAgents.push_back(m_holeDrainAgentRight);

    // Exciton drain
    m_recombinationAgent = new RecombinationAgent(refWorld, this);
    m_drains.push_back(m_recombinationAgent);
    m_xDrains.push_back(m_recombinationAgent);
    m_fluxAgents.push_back(m_recombinationAgent);

    if (parameters().simulationType == "transistor")
    {
        // Turn off recombination
        m_recombinationAgent->setRate(0.0);

        // Set rates
        m_electronDrainAgentRight->setRateSmartly(parameters().eDrainRRate, parameters().drainRate);
        m_electronDrainAgentLeft->setRateSmartly(parameters().eDrainLRate, 0.0);
        m_holeDrainAgentRight->setRateSmartly(parameters().hDrainRRate, 0.0);
        m_holeDrainAgentLeft->setRateSmartly(parameters().hDrainLRate, 0.0);
    }
    else if (parameters().simulationType == "solarcell")
    {
        // Turn recombination on
        m_recombinationAgent->setRate(parameters().recombinationRate);

        // Set rates
        m_electronDrainAgentRight->setRateSmartly(parameters().eDrainRRate, parameters().drainRate);
        m_electronDrainAgentLeft->setRateSmartly(parameters().eDrainLRate, parameters().drainRate);
        m_holeDrainAgentRight->setRateSmartly(parameters().hDrainRRate, parameters().drainRate);
        m_holeDrainAgentLeft->setRateSmartly(parameters().hDrainLRate, parameters().drainRate);
    }
    else
    {
        qFatal("langmuir: unknown simulation.type - can not create drains");
    }

    qDebug("langmuir: e.drain.r.rate = %.3g", m_electronDrainAgentRight->rate());
    qDebug("langmuir: e.drain.l.rate = %.3g", m_electronDrainAgentLeft->rate());
    qDebug("langmuir: h.drain.r.rate = %.3g", m_holeDrainAgentRight->rate());
    qDebug("langmuir: h.drain.l.rate = %.3g", m_holeDrainAgentLeft->rate());
    qDebug("langmuir: recombination.rate = %.3g", m_recombinationAgent->rate());

    if ((m_electronDrainAgentRight->rate() == 0) &&
        (m_electronDrainAgentLeft->rate() == 0) &&
        (m_holeDrainAgentRight->rate() == 0) &&
        (m_holeDrainAgentLeft->rate() == 0) &&
        (m_recombinationAgent->rate() == 0))
    {
        qFatal("langmuir: all drains have zero rate");
    }

    if (maxElectronAgents() > 0)
    {
        if ((m_electronDrainAgentRight->rate() <= 0) &&
            (m_electronDrainAgentLeft->rate() <= 0) &&
            (m_recombinationAgent->rate() <= 0))
        {
            qFatal("langmuir: all electron drain rates are zero, yet maxElectronAgents() == %d",
                   maxElectronAgents());
        }
    }

    if (maxHoleAgents() > 0)
    {
        if ((m_holeDrainAgentRight->rate() <= 0) &&
            (m_holeDrainAgentLeft->rate() <= 0) &&
            (m_recombinationAgent->rate() <= 0))
        {
            qFatal("langmuir: all hole drain rates are zero, yet maxElectronAgents() == %d",
                   maxElectronAgents());
        }
    }
}

void World::setFluxInfo(const QList<quint64> &fluxInfo)
{
    qDebug("langmuir: World::setFluxInfo");

    if (fluxInfo.size() == 0)
    {
        return;
    }

    QString format; QTextStream stream(&format);
    stream << "Legacy checkpoint file format (Transistor)" << '\n';
    stream << "[FluxInfo]" << '\n';
    stream << "0 : number of entries (=4)" << '\n';
    stream << "1 : ESL attempt" << '\n';
    stream << "2 : ESL success" << '\n';
    stream << "3 : EDR attempt" << '\n';
    stream << "4 : EDL success" << '\n';
    stream << '\n';
    stream << "Legacy checkpoint file format (SolarCell)" << '\n';
    stream << "[FluxInfo]" << '\n';
    stream << "0 : number of entries (=10..12)" << '\n';
    stream << "1 : XS  attempt" << '\n';
    stream << "2 : XS  success" << '\n';
    stream << "3 : EDL attempt" << '\n';
    stream << "4 : EDL success" << '\n';
    stream << "5 : HDL attempt" << '\n';
    stream << "6 : HDL success" << '\n';
    stream << "7 : EDR attempt" << '\n';
    stream << "8 : EDR success" << '\n';
    stream << "9 : HDR attempt" << '\n';
    stream << "10: HDR success" << '\n';
    stream << "11: XD  attempt (may not be present)" << '\n';
    stream << "12: XD  success (may not be present)" << '\n';
    stream << '\n';
    stream << "Current checkpoint file format (Both)" << '\n';
    stream << "[FluxInfo]" << '\n';
    stream << "0 : number of entries (=10..12)" << '\n';
    stream << "1 : ESL attempt" << '\n';
    stream << "2 : ESL success" << '\n';
    stream << "3 : ESR attempt" << '\n';
    stream << "4 : ESR success" << '\n';
    stream << "5 : HSL attempt" << '\n';
    stream << "6 : HSL success" << '\n';
    stream << "7 : HSR attempt" << '\n';
    stream << "8 : HSR success" << '\n';
    stream << "9 : XS  attempt" << '\n';
    stream << "10: XS  success" << '\n';
    stream << "11: EDL attempt" << '\n';
    stream << "12: EDL success" << '\n';
    stream << "13: EDR attempt" << '\n';
    stream << "14: EDR success" << '\n';
    stream << "15: HDL attempt" << '\n';
    stream << "16: HDL success" << '\n';
    stream << "17: HDR attempt" << '\n';
    stream << "18: HDR success" << '\n';
    stream << "19: XD  attempt" << '\n';
    stream << "20: XD  success" << '\n';
    stream.flush();

    if (fluxInfo.size() == 4 && parameters().simulationType == "transistor")
    {
        m_electronSourceAgentLeft->setAttempts(fluxInfo[0]);
        m_electronSourceAgentLeft->setSuccesses(fluxInfo[1]);

        m_electronDrainAgentRight->setAttempts(fluxInfo[2]);
        m_electronDrainAgentRight->setSuccesses(fluxInfo[3]);
    }
    else if ((fluxInfo.size() == 10 || fluxInfo.size() == 12) &&
             parameters().simulationType == "solarcell")
    {
        m_excitonSourceAgent->setAttempts(fluxInfo[0]);
        m_excitonSourceAgent->setSuccesses(fluxInfo[1]);

        m_electronDrainAgentLeft->setAttempts(fluxInfo[2]);
        m_electronDrainAgentLeft->setSuccesses(fluxInfo[3]);

        m_holeDrainAgentLeft->setAttempts(fluxInfo[4]);
        m_holeDrainAgentLeft->setSuccesses(fluxInfo[5]);

        m_electronDrainAgentRight->setAttempts(fluxInfo[6]);
        m_electronDrainAgentRight->setSuccesses(fluxInfo[7]);

        m_holeDrainAgentRight->setAttempts(fluxInfo[8]);
        m_holeDrainAgentRight->setSuccesses(fluxInfo[9]);

        if (fluxInfo.size() == 12)
        {
            m_recombinationAgent->setAttempts(fluxInfo[10]);
            m_recombinationAgent->setSuccesses(fluxInfo[11]);
        }
    }
    else if (fluxInfo.size() == 20)
    {
        m_electronSourceAgentLeft->setAttempts(fluxInfo[0]);
        m_electronSourceAgentLeft->setSuccesses(fluxInfo[1]);

        m_electronSourceAgentRight->setAttempts(fluxInfo[2]);
        m_electronSourceAgentRight->setSuccesses(fluxInfo[3]);

        m_holeSourceAgentLeft->setAttempts(fluxInfo[4]);
        m_holeSourceAgentLeft->setSuccesses(fluxInfo[5]);

        m_holeSourceAgentRight->setAttempts(fluxInfo[6]);
        m_holeSourceAgentRight->setSuccesses(fluxInfo[7]);

        m_excitonSourceAgent->setAttempts(fluxInfo[8]);
        m_excitonSourceAgent->setSuccesses(fluxInfo[9]);

        m_electronDrainAgentLeft->setAttempts(fluxInfo[10]);
        m_electronDrainAgentLeft->setSuccesses(fluxInfo[11]);

        m_electronDrainAgentRight->setAttempts(fluxInfo[12]);
        m_electronDrainAgentRight->setSuccesses(fluxInfo[13]);

        m_holeDrainAgentLeft->setAttempts(fluxInfo[14]);
        m_holeDrainAgentLeft->setSuccesses(fluxInfo[15]);

        m_holeDrainAgentRight->setAttempts(fluxInfo[16]);
        m_holeDrainAgentRight->setSuccesses(fluxInfo[17]);

        m_recombinationAgent->setAttempts(fluxInfo[18]);
        m_recombinationAgent->setSuccesses(fluxInfo[19]);
    }
    else
    {
        qFatal("langmuir: %s\ndo not recognize [FluxInfo] format... (of size %d, for simulation.type=%s)",
               qPrintable(format), fluxInfo.size(), qPrintable(parameters().simulationType));
    }
}

}
