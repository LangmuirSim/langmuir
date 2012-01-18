#include "simulation.h"
#include "openclhelper.h"
#include "inputparser.h"
#include "chargeagent.h"
#include "sourceagent.h"
#include "drainagent.h"
#include "potential.h"
#include "cubicgrid.h"
#include "logger.h"
#include "world.h"
#include "rand.h"
namespace Langmuir
{
Simulation::Simulation(SimulationParameters * par, QObject *parent):  QObject(parent), m_world(0)
{
    // Create the World Object
    m_world = new World(par);

    // Place Defects
    this->createDefects();

    // Setup Grid Potential
    // Zero potential
    m_world->potential()->setPotentialZero();

    // Add Linear Potential
    m_world->potential()->setPotentialLinear();

    // Add Trap Potential(assumes source and drain were created so that hetero traps don't start growing on the source / drain)
    m_world->potential()->setPotentialTraps();

    // precalculate and store coulomb interaction energies
    m_world->potential()->updateInteractionEnergies();

    // place charges on the grid randomly(assumes source / drain were created)
    if(m_world->parameters()->seedCharges)seedCharges();

    // Generate grid image
    if(m_world->parameters()->outputTrapImage)
    {
        m_world->logger()->saveTrapImage();
    }

    // Output Field Energy
    if(m_world->parameters()->outputElectronGrid)
    {
        m_world->logger()->saveElectronGridPotential();
    }
    if(m_world->parameters()->outputHoleGrid)
    {
        m_world->logger()->saveHoleGridPotential();
    }

    // Output Defect IDs
    if(m_world->parameters()->outputDefectIDs)
    {
        m_world->logger()->saveDefectIDs();
    }

    // Output Trap IDs
    if(m_world->parameters()->outputTrapIDs)
    {
        m_world->logger()->saveTrapIDs();
    }

    // Initialize OpenCL
    m_world->opencl()->initializeOpenCL();
    if(m_world->parameters()->useOpenCL)
    {
        m_world->opencl()->toggleOpenCL(true);
    }
    else
    {
        m_world->opencl()->toggleOpenCL(false);
    }
}

Simulation::~Simulation()
{
    if(m_world != 0)
    {
        delete m_world;
    }
}

void Simulation::performIterations(int nIterations)
{
    //m_world->electrons()->at(0)->coulombInteraction(m_world->electrons()->at(0)->site(true));
    //  cout << "Entered performIterations function.\n";
    for(int i = 0; i < nIterations; ++i)
    {
        // Attempt to transport the charges through the film
        QList < ChargeAgent * >&electrons = *m_world->electrons();
        QList < ChargeAgent * >&holes = *m_world->holes();

        // If using OpenCL, launch the Kernel to calculate Coulomb Interactions
        if(m_world->parameters()->useOpenCL &&(electrons.size()> 0 || holes.size()> 0))
        {
            // first have the charge carriers propose future sites
            //for(int j = 0; j < charges.size(); j++)charges[j]->chooseFuture(j);
            QFuture < void > future1 = QtConcurrent::map(electrons, Simulation::chargeAgentChooseFuture);
            QFuture < void > future2 = QtConcurrent::map(holes, Simulation::chargeAgentChooseFuture);
            future1.waitForFinished();
            future2.waitForFinished();

            // tell the GPU to perform all coulomb calculations
            m_world->opencl()->launchCoulombKernel2();

            //check the answer during debugging
            //m_world->opencl()->compareHostAndDeviceForAllCarriers();
            //qFatal("done");

            // now use the results of the coulomb calculations to decide if the carreirs should have moved
            future1 = QtConcurrent::map(electrons, Simulation::chargeAgentDecideFuture);
            future2 = QtConcurrent::map(holes, Simulation::chargeAgentDecideFuture);
            future1.waitForFinished();
            future2.waitForFinished();
        }
        else // Not using OpenCL
        {
            // Use QtConcurrnet to parallelise the charge calculations
            QFuture < void > future1 = QtConcurrent::map(electrons, Simulation::chargeAgentIterate);
            QFuture < void > future2 = QtConcurrent::map(holes, Simulation::chargeAgentIterate);

            // We want to wait for it to finish before continuing on
            future1.waitForFinished();
            future2.waitForFinished();
        }

        // Now we are done with the charge movement, move them to the next tick!
        nextTick();

        // Perform charge injection at the source
        performInjections();

        m_world->parameters()->currentStep += 1;
    }

    // Output Coulomb Energy
    if(m_world->parameters()->outputCoulomb)
    {
        m_world->opencl()->launchCoulombKernel1();
        m_world->logger()->saveCoulombEnergy();
    }
    // Output Carrier Positions and IDs
    if(m_world->parameters()->outputElectronIDs)
    {
        m_world->logger()->saveElectronIDs();
    }
    if(m_world->parameters()->outputHoleIDs)
    {
        m_world->logger()->saveHoleIDs();
    }
}

void Simulation::createDefects()
{
    for(int i = 0; i < m_world->electronGrid()->volume(); ++i)
    {
        if(m_world->randomNumberGenerator()->random()< m_world->parameters()->defectPercentage)
        {
            m_world->electronGrid()->registerDefect(i);
            m_world->holeGrid()->registerDefect(i);
            m_world->defectSiteIDs()->push_back(i);
        }
    }
}

void Simulation::seedCharges()
{
    SourceAgent * eSource = new ElectronSourceAgent(m_world,Grid::NoFace,0,1.0,1,0);
    SourceAgent * pSource = new HoleSourceAgent(m_world,Grid::NoFace,0,1.0,1,0);

    for(int i = 0; i < eSource->maxElectrons();)
    {
        if(eSource->seed()) { ++i; }
    }

    for(int i = 0; i < pSource->maxHoles();)
    {
        if(pSource->seed()) { ++i; }
    }

    delete eSource;
    delete pSource;
}

void Simulation::performInjections()
{
    for(int i = 0; i < m_world->sources()->size(); i++)
    {
        m_world->sources()->at(i)->tryToInject();
    }
}

void Simulation::nextTick()
{
    // Iterate over all sites to change their state
    QList < ChargeAgent * >&electrons = *m_world->electrons();
    QList < ChargeAgent * >&holes = *m_world->holes();

    if ( m_world->parameters()->outputOnDelete )
    {
        for(int i = 0; i < electrons.size(); ++i)
        {
            electrons[i]->completeTick();
            // Check if the charge was removed - then we should delete it
            if(electrons[i]->removed())
            {
                m_world->logger()->report(*electrons[i]);
                delete electrons[i];
                electrons.removeAt(i);
                --i;
            }
        }
        for(int i = 0; i < holes.size(); ++i)
        {
            holes[i]->completeTick();
            // Check if the charge was removed - then we should delete it
            if(holes[i]->removed())
            {
                m_world->logger()->report(*holes[i]);
                delete holes[i];
                holes.removeAt(i);
                --i;
            }
        }
        m_world->logger()->flush();
    }
    else
    {
        for(int i = 0; i < electrons.size(); ++i)
        {
            electrons[i]->completeTick();
            // Check if the charge was removed - then we should delete it
            if(electrons[i]->removed())
            {
                delete electrons[i];
                electrons.removeAt(i);
                --i;
            }
        }
        for(int i = 0; i < holes.size(); ++i)
        {
            holes[i]->completeTick();
            // Check if the charge was removed - then we should delete it
            if(holes[i]->removed())
            {
                delete holes[i];
                holes.removeAt(i);
                --i;
            }
        }
    }
}

inline void Simulation::chargeAgentIterate(ChargeAgent * chargeAgent)
{
    // This function performs a single iteration for a charge agent, only thread
    // safe calls can be made in this function. Other threads may access the
    // current state of the chargeAgent, but will not attempt to modify it.
    chargeAgent->chooseFuture();
    chargeAgent->decideFuture();
}

inline void Simulation::chargeAgentChooseFuture(ChargeAgent * chargeAgent)
{
    // same rules apply here as for chargeAgentIterate;
    chargeAgent->chooseFuture();
}

inline void Simulation::chargeAgentDecideFuture(ChargeAgent * chargeAgent)
{
    // same rules apply here as for chargeAgentIterate;
    chargeAgent->decideFuture();
}

}
