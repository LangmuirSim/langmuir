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

namespace Langmuir {

World::World(const QString &fileName, QObject *parent)
    : QObject(parent),
      m_keyValueParser(0),
      m_checkPointer(0),
      m_excitonSourceAgent(0),
      m_recombinationAgent(0),
      m_electronGrid(0),
      m_holeGrid(0),
      m_rand(0),
      m_potential(0),
      m_parameters(0),
      m_logger(0),
      m_ocl(0),
      m_maxElectrons(0),
      m_maxHoles(0),
      m_maxDefects(0),
      m_maxTraps(0)
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

ExcitonSourceAgent& World::excitonSourceAgent()
{
    return *m_excitonSourceAgent;
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
    // keyValueParser is an object that maps between parameter
    // names and their locations in the simulationParameters struct
    // It is also the global location of the simulationParameters struct,
    // that a lot of objects carry around references to
    m_keyValueParser = new KeyValueParser(refWorld,this);

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
    m_checkPointer = new CheckPointer(refWorld,this);

    // Create Site info
    // This is temporary storage for info (like electron locations)
    // loaded in the simulation input file
    ConfigurationInfo configInfo;

    // Parse the input file
    m_checkPointer->load(fileName,configInfo);
    checkSimulationParameters(*m_parameters);

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

    // Create the Sources and Drains
    if ( parameters().simulationType == "transistor" )
    {
        // Create Transistor Sources
        m_sources.push_back(new ElectronSourceAgent(refWorld, Grid::Left, this));
        m_eSources.push_back(m_sources.last());
        m_sources.last()->setPotential(parameters().voltageLeft);
        m_sources.last()->setRate(parameters().sourceRate);

        // Create Transistor Drains
        m_drains.push_back(new ElectronDrainAgent(refWorld, Grid::Right, this));
        m_eDrains.push_back(m_drains.last());
        m_drains.last()->setPotential(parameters().voltageRight);
        if (parameters().eDrainRRate >= 0)
        {
            qWarning("warning: e.drain.r.rate=(%.3f) has been overridden drain.rate=(%.3e)"
                     "for eDrainR in transistor",
                     parameters().eDrainRRate, parameters().drainRate);
            m_drains.last()->setRate(parameters().eDrainRRate);
        }
        else
        {
            m_drains.last()->setRate(parameters().drainRate);
        }
    }
    else if ( parameters().simulationType == "solarcell" )
    {
        // Create Solar Cell Sources
        m_excitonSourceAgent = new ExcitonSourceAgent(refWorld, this);
        m_sources.push_back(m_excitonSourceAgent);
        m_xSources.push_back(m_sources.last());
        m_sources.last()->setRate(parameters().sourceRate);

        // Scale the source.rate according to the area of the simulation cell
        if ( parameters().sourceScaleArea > 0 && parameters().sourceScaleArea != m_electronGrid->xyPlaneArea())
        {
            double oldRate = parameters().sourceRate;
            double scaleFactor = m_electronGrid->xyPlaneArea() / double(parameters().sourceScaleArea);
            double newRate = oldRate * scaleFactor;
            qWarning("warning: source.rate is being scaled... %.5e * 2^(%.5e) = %.5e",
                     oldRate, log2(scaleFactor), newRate);
            m_sources.last()->setRate(newRate);
        }

        // Create Solar Cell Drains
        m_drains.push_back(new ElectronDrainAgent(refWorld, Grid::Left, this));
        m_eDrains.push_back(m_drains.last());
        m_drains.last()->setPotential(parameters().voltageLeft);
        if (parameters().eDrainLRate >= 0)
        {
            qWarning("warning: e.drain.l.rate=%.3e has been overridden drain.rate=%.3e"
                     " for the left electron drain in solar cell",
                     parameters().eDrainLRate, parameters().drainRate);
            m_drains.last()->setRate(parameters().eDrainLRate);
        }
        else
        {
            m_drains.last()->setRate(parameters().drainRate);
        }

        m_drains.push_back(new HoleDrainAgent(refWorld, Grid::Left, this));
        m_hDrains.push_back(m_drains.last());
        m_drains.last()->setPotential(parameters().voltageLeft);
        if (parameters().hDrainLRate >= 0)
        {
            qWarning("warning: h.drain.l.rate=%.3e has been overridden drain.rate=%.3e"
                     " for the left hole drain in solar cell",
                     parameters().hDrainLRate, parameters().drainRate);
            m_drains.last()->setRate(parameters().hDrainLRate);
        }
        else
        {
            m_drains.last()->setRate(parameters().drainRate);
        }

        m_drains.push_back(new ElectronDrainAgent(refWorld, Grid::Right, this));
        m_eDrains.push_back(m_drains.last());
        m_drains.last()->setPotential(parameters().voltageRight);
        if (parameters().eDrainRRate >= 0)
        {
            qWarning("warning: e.drain.r.rate=%.3e has been overridden drain.rate=%.3e"
                     " for the right electron drain in solar cell",
                     parameters().eDrainRRate, parameters().drainRate);
            m_drains.last()->setRate(parameters().eDrainRRate);
        }
        else
        {
            m_drains.last()->setRate(parameters().drainRate);
        }

        m_drains.push_back(new HoleDrainAgent(refWorld, Grid::Right, this));
        m_hDrains.push_back(m_drains.last());
        m_drains.last()->setPotential(parameters().voltageRight);
        if (parameters().hDrainRRate >= 0)
        {
            qWarning("warning: h.drain.r.rate=%.3e has been overridden drain.rate=%.3e"
                     " for the right hole drain in solar cell",
                     parameters().hDrainRRate, parameters().drainRate);
            m_drains.last()->setRate(parameters().hDrainRRate);
        }
        else
        {
            m_drains.last()->setRate(parameters().drainRate);
        }

        // Create Recombination Agent
        m_recombinationAgent = new RecombinationAgent(refWorld, this);
        m_drains.push_back(m_recombinationAgent);
        m_xDrains.push_back(m_drains.last());
        m_drains.last()->setRate(parameters().recombinationRate);
        m_drains.last()->setPotential(0.0);
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

    // Set previous flux state
    if (configInfo.fluxInfo.size() > 0)
    {
        if (configInfo.fluxInfo.size() % 2 != 0 ||
            configInfo.fluxInfo.size() != 2*m_fluxAgents.size())
        {
            qWarning("warning: [FluxInfo] has an invalid size for this simulation.type (expected %d values, found %d)",
                     2*m_fluxAgents.size(), configInfo.fluxInfo.size());
            if (configInfo.fluxInfo.size() > 2*m_fluxAgents.size())
            {
                qWarning("warning: the excess values will be ignored...");
            }
            else
            {
                qWarning("warning: missing values will be set to zero...");
            }
        }
        int j = 0;
        for (int i = 0; i < m_fluxAgents.size(); i++)
        {
            if (j < configInfo.fluxInfo.size())
            {
                m_fluxAgents.at(i)->setAttempts(configInfo.fluxInfo.at(j));
            }
            else
            {
                qWarning("warning: missing [FluxInfo] attempts for fluxAgent %s",
                         qPrintable(m_fluxAgents.at(i)->objectName()));
            }

            if ((j + 1) < configInfo.fluxInfo.size())
            {
                m_fluxAgents.at(i)->setSuccesses(configInfo.fluxInfo.at(j+1));
            }
            else
            {
                qWarning("warning: missing [FluxInfo] successes for fluxAgent %s",
                         qPrintable(m_fluxAgents.at(i)->objectName()));
            }
            j += 2;
        }
    }

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
    potential().updateInteractionEnergies();

    // precalculate and store coupling constants
    potential().updateCouplingConstants();

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
