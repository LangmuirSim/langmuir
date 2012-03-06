#include "simulation.h"
#include "openclhelper.h"
#include "parameters.h"
#include "chargeagent.h"
#include "sourceagent.h"
#include "drainagent.h"
#include "potential.h"
#include "cubicgrid.h"
#include "writer.h"
#include "world.h"
#include "rand.h"
namespace Langmuir
{

Simulation::Simulation(World &world, QObject *parent):  QObject(parent), m_world(world)
{
}

Simulation::~Simulation()
{
}

void Simulation::performIterations(int nIterations)
{
    for(int i = 0; i < nIterations; ++i)
    {
        // Attempt to transport the charges through the film
        QList<ChargeAgent*> &electrons = m_world.electrons();
        QList<ChargeAgent*> &holes = m_world.holes();

        // If using OpenCL, launch the Kernel to calculate Coulomb Interactions
        if(m_world.parameters().useOpenCL &&(electrons.size()> 0 || holes.size()> 0))
        {
            // first have the charge carriers propose future sites
            //for(int j = 0; j < charges.size(); j++)charges[j]->chooseFuture(j);
            QFuture < void > future1 = QtConcurrent::map(electrons, Simulation::chargeAgentChooseFuture);
            QFuture < void > future2 = QtConcurrent::map(holes, Simulation::chargeAgentChooseFuture);
            future1.waitForFinished();
            future2.waitForFinished();

            // tell the GPU to perform all coulomb calculations
            m_world.opencl().launchCoulombKernel2();

            //check the answer during debugging
            //m_world.opencl().compareHostAndDeviceForAllCarriers();
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

        m_world.parameters().currentStep += 1;
    }

    // Output Source and Drain information
    m_world.logger().reportFluxStream();

    // Output Coulomb Energy
    if(m_world.parameters().outputCoulomb)
    {
        m_world.opencl().launchCoulombKernel1();
        m_world.logger().saveCoulombEnergy();
    }

    if(m_world.parameters().outputXyz)
    {
        m_world.logger().reportXYZStream();
    }
}

void Simulation::performInjections()
{
    for(int i = 0; i < m_world.sources().size(); i++)
    {
        m_world.sources().at(i)->tryToInject();
    }
}

void Simulation::nextTick()
{
    // Iterate over all sites to change their state
    QList<ChargeAgent*> &electrons = m_world.electrons();
    QList<ChargeAgent*> &holes = m_world.holes();

    if ( m_world.parameters().outputIdsOnDelete )
    {
        for(int i = 0; i < electrons.size(); ++i)
        {
            electrons[i]->completeTick();
            // Check if the charge was removed - then we should delete it
            if(electrons[i]->removed())
            {
                m_world.logger().reportCarrier(*electrons[i]);
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
                m_world.logger().reportCarrier(*holes[i]);
                delete holes[i];
                holes.removeAt(i);
                --i;
            }
        }
        //m_world.logger().flush();
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
