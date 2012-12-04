#include "simulation.h"
#include "openclhelper.h"
#include "parameters.h"
#include "chargeagent.h"
#include "sourceagent.h"
#include "drainagent.h"
#include "potential.h"
#include "cubicgrid.h"
#include "checkpointer.h"
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
    // Do some parallel stuff if using Coulomb interactions
    if (m_world.parameters().coulombCarriers)
    {
        for(int i = 0; i < nIterations; ++i)
        {
            QList<ChargeAgent*> &electrons = m_world.electrons();
            QList<ChargeAgent*> &holes = m_world.holes();

            // Select future sites in serial (because random number generator is being used)
            for (int i = 0; i < electrons.size(); i++)
            {
                electrons.at(i)->chooseFuture();
            }
            for (int i = 0; i < holes.size(); i++)
            {
                holes.at(i)->chooseFuture();
            }

            // Calculate the coulomb interactions in parallel some way or another
            if (m_world.parameters().useOpenCL && m_world.numChargeAgents() > m_world.parameters().openclThreshold)
            {
                    // Use OpenCL if there are a lot of charges
                    m_world.opencl().launchCoulombKernel2();

                    QFutureSynchronizer<void> sync;
                    sync.addFuture(QtConcurrent::map(electrons, Simulation::chargeAgentCoulombInteractionQtConcurrentGPU));
                    sync.addFuture(QtConcurrent::map(holes, Simulation::chargeAgentCoulombInteractionQtConcurrentGPU));
                    sync.waitForFinished();
            }
            else
            {
                // Use multi threaded CPU if there are not many charges or when we can not use OpenCL
                QFutureSynchronizer<void> sync;
                sync.addFuture(QtConcurrent::map(electrons, Simulation::chargeAgentCoulombInteractionQtConcurrentCPU));
                sync.addFuture(QtConcurrent::map(holes, Simulation::chargeAgentCoulombInteractionQtConcurrentCPU));
                sync.waitForFinished();
            }

            // Decide future in serial (because random number generator is being used)
            for (int i = 0; i < electrons.size(); i++)
            {
                electrons.at(i)->decideFuture();
            }
            for (int i = 0; i < holes.size(); i++)
            {
                holes.at(i)->decideFuture();
            }

            // Recombine holes and electrons
            performRecombinations();

            // Now we are done with the charge movement, move them to the next tick!
            nextTick();

            // Perform charge injection at the source
            performInjections();

            m_world.parameters().currentStep += 1;
        }
    }

    // Skip the parallel stuff if no Coulomb interactions
    else
    {
        for(int i = 0; i < nIterations; ++i)
        {
            QList<ChargeAgent*> &electrons = m_world.electrons();
            QList<ChargeAgent*> &holes = m_world.holes();

            // Select future sites in serial (because random number generator is being used)
            for (int i = 0; i < electrons.size(); i++)
            {
                electrons.at(i)->chooseFuture();
            }
            for (int i = 0; i < holes.size(); i++)
            {
                holes.at(i)->chooseFuture();
            }

            // Decide future in serial (because random number generator is being used)
            for (int i = 0; i < electrons.size(); i++)
            {
                electrons.at(i)->decideFuture();
            }
            for (int i = 0; i < holes.size(); i++)
            {
                holes.at(i)->decideFuture();
            }

            // Recombine holes and electrons
            performRecombinations();

            // Now we are done with the charge movement, move them to the next tick!
            nextTick();

            // Perform charge injection at the source
            performInjections();

            m_world.parameters().currentStep += 1;
        }
    }

    // Update RecombinationAgent probability
    if (m_world.parameters().simulationType == "solarcell")
    {
        m_world.recombinationAgent().guessProbability();
    }

    // Output Source and Drain information
    m_world.logger().reportFluxStream();

    // Output Coulomb Energy
    if ( m_world.numChargeAgents() > 0 &&
         m_world.parameters().outputCoulomb > 0 &&
         m_world.parameters().currentStep %
        (m_world.parameters().iterationsPrint *
         m_world.parameters().outputCoulomb) == 0
       )
    {
        m_world.opencl().launchCoulombKernel1();
        m_world.logger().saveCoulombEnergy();
    }

    // Output Trajectory
    if ( m_world.parameters().outputXyz > 0 &&
         m_world.parameters().currentStep %
        (m_world.parameters().iterationsPrint *
         m_world.parameters().outputXyz) == 0
       )
    {
        m_world.logger().reportXYZStream();
    }

    // Output Image of Carriers
    if ( m_world.parameters().imageCarriers > 0 &&
         m_world.parameters().currentStep %
        (m_world.parameters().iterationsPrint *
         m_world.parameters().imageCarriers) == 0
       )
    {
        m_world.logger().saveCarriersImage();
        m_world.logger().saveElectronImage();
        m_world.logger().saveHoleImage();
    }

    // Save a Checkpoint Files
    if (m_world.parameters().outputIsOn)
    {
        // For a specific step
        if ( m_world.parameters().outputStepChk > 0 &&
             m_world.parameters().currentStep %
            (m_world.parameters().iterationsPrint *
             m_world.parameters().outputStepChk) == 0
           )
        {
            //m_world.checkPointer().save("%stub-%step.chk");
            m_world.checkPointer().save();
        }
    }
}

void Simulation::performRecombinations()
{
    if (m_world.parameters().recombinationRate > 0)
    {
        if (m_world.parameters().outputIdsOnEncounter)
        {
            // recombine and output ids
            foreach (ChargeAgent *charge, m_world.electrons())
            {
                int site = charge->getCurrentSite();
                if (charge->otherGrid().agentType(site) == charge->otherType())
                {
                    ChargeAgent *other = dynamic_cast<ChargeAgent*>(charge->otherGrid().agentAddress(site));
                    if (other)
                    {
                        if (m_world.recombinationAgent().tryToAccept(charge))
                        {
                            charge->setRemoved(true);
                            other->setRemoved(true);
                            m_world.logger().reportExciton(*charge, *other, true);
                        }
                        else
                        {
                            m_world.logger().reportExciton(*charge, *other, false);
                        }
                    }
                    else
                    {
                        qFatal("dynamic cast from Agent* to ChargeAgent* has failed during recombination");
                    }
                }
            } // end loop
        }
        else
        {
            // just recombine
            foreach (ChargeAgent *charge, m_world.electrons())
            {
                int site = charge->getCurrentSite();
                if (charge->otherGrid().agentType(site) == charge->otherType())
                {
                    ChargeAgent *other = dynamic_cast<ChargeAgent*>(charge->otherGrid().agentAddress(site));
                    if (other)
                    {
                        if (m_world.recombinationAgent().tryToAccept(charge))
                        {
                            charge->setRemoved(true);
                            other->setRemoved(true);
                        }
                    }
                    else
                    {
                        qFatal("dynamic cast from Agent* to ChargeAgent* has failed during recombination");
                    }
                }
            } // end loop
        }
    }
    else
    {
        if (m_world.parameters().outputIdsOnEncounter)
        {
            // just output ids
            foreach (ChargeAgent *charge, m_world.electrons())
            {
                int site = charge->getCurrentSite();
                if (charge->otherGrid().agentType(site) == charge->otherType())
                {
                    ChargeAgent *other = dynamic_cast<ChargeAgent*>(charge->otherGrid().agentAddress(site));
                    if (other)
                    {
                        m_world.logger().reportExciton(*charge, *other, false);
                    }
                    else
                    {
                        qFatal("dynamic cast from Agent* to ChargeAgent* has failed during recombination");
                    }
                }
            } // end loop
        }
        else
        {
            // do nothing
            return;
        }
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

inline void Simulation::chargeAgentCoulombInteractionQtConcurrentCPU(ChargeAgent * chargeAgent)
{
    chargeAgent->coulombCPU();
}

inline void Simulation::chargeAgentCoulombInteractionQtConcurrentGPU(ChargeAgent * chargeAgent)
{
    chargeAgent->coulombGPU();
}

}
