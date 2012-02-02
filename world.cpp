#include "parameters.h"
#include "openclhelper.h"
#include "chargeagent.h"
#include "sourceagent.h"
#include "drainagent.h"
#include "potential.h"
#include "cubicgrid.h"
#include "logger.h"
#include "world.h"
#include "rand.h"
#include "fluxagent.h"

namespace Langmuir {

World::World(SimulationParameters &par, QObject *parent)
    : QObject(parent)
{
    // Set the address of the Parameters passed
    m_parameters = &par;

    // Pointers are EVIL
    World &refWorld = *this;

    // Create Random Number Generator
    m_rand = new Random(parameters().randomSeed);

    // Create Electron Grid
    m_electronGrid = new Grid(parameters().gridX, parameters().gridY, parameters().gridZ, this);

    // Create Hole Grid
    m_holeGrid = new Grid(parameters().gridX, parameters().gridY, parameters().gridZ, this);

    // Calculate the max number of holes
    m_maxHoles = parameters().holePercentage*double(holeGrid().volume());

    // Calculate the max number of electrons
    m_maxElectrons = parameters().electronPercentage*double(electronGrid().volume());

    // Create Potential Calculator
    m_potential = new Potential(refWorld, this);

    // Create OpenCL Objects
    m_ocl = new OpenClHelper(refWorld, this);

    // Create Transistor Sources
    m_sources.push_back(new ElectronSourceAgent(refWorld, Grid::Left, this));
    m_sources.last()->setRate(0.90);

    // Create Transistor Drains
    m_drains.push_back(new ElectronDrainAgent(refWorld, Grid::Right, this));
    m_drains.last()->setRate(0.90);

    // Create Solar Cell Sources
    //m_sources.push_back(new ExcitonSourceAgent(refWorld, this));
    //m_sources.last()->setRate(0.001);

    // Create Solar Cell Drains
    //m_drains.push_back(new ElectronDrainAgent(refWorld, Grid::Left, this));
    //m_drains.last()->setRate(0.90);

    //m_drains.push_back(new HoleDrainAgent(refWorld, Grid::Left, this));
    //m_drains.last()->setRate(0.90);

    //m_drains.push_back(new ElectronDrainAgent(refWorld, Grid::Right, this));
    //m_drains.last()->setRate(0.90);

    //m_drains.push_back(new HoleDrainAgent(refWorld, Grid::Right, this));
    //m_drains.last()->setRate(0.90);

    foreach( FluxAgent *flux, m_sources )
    {
        m_fluxAgents.push_back(flux);
    }

    foreach( FluxAgent *flux, m_drains )
    {
        m_fluxAgents.push_back(flux);
    }

    // Create Logger
    m_logger = new Logger(refWorld, this);
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

double World::percentChargeAgents()
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

double World::percentHoleAgents()
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

double World::percentElectronAgents()
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

}
