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
            //Store fluxAgent states
            foreach (FluxAgent* flux, m_world.fluxes())
            {
                flux->storeLast();
            }

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
                if (m_world.parameters().coulombGaussianSigma > 0)
                {
                    m_world.opencl().launchGaussKernel2();
                }
                else
                {
                    m_world.opencl().launchCoulombKernel2();
                }

                // Turn this on to check the GPU vs CPU
                // TRUST THE GPU - if it gives the wrong answer it is most likely
                // the information passed to it is wrong some how, or something was
                // changed in the CPU version. There is like a 99.9999% chance
                // something is messed up on the CPU side - I have spent days/hours
                // being tormented by some sublte bug, and it always turns out to
                // be something wrong with the CPU functions
                // m_world.opencl().compareHostAndDeviceForAllCarriers();

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
            //Store fluxAgent states
            foreach (FluxAgent* flux, m_world.fluxes())
            {
                flux->storeLast();
            }

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

    // Save output
    if (m_world.parameters().outputIsOn)
    {
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

        // Output checkpoint file
        if ( m_world.parameters().outputStepChk > 0 &&
             m_world.parameters().currentStep %
            (m_world.parameters().iterationsPrint *
             m_world.parameters().outputStepChk) == 0
           )
        {
            m_world.checkPointer().save();
        }
    }
}

void Simulation::performRecombinations()
{
    if (m_world.parameters().simulationType == "solarcell")
    {
        if (m_world.parameters().recombinationRate > 0)
        {
            foreach (ChargeAgent *charge, m_world.electrons())
            {
                m_world.recombinationAgent().tryToAccept(charge);
            }
        }
    }
}

void Simulation::performInjections()
{
    if (m_world.parameters().simulationType == "solarcell")
    {
        m_world.excitonSourceAgent().tryToInject();

        if (m_world.parameters().balanceCharges)
        {
            balanceCharges();
        }
    }
    else
    {
        m_world.electronSourceAgentLeft().tryToInject();
    }
}

void Simulation::balanceCharges()
{
    int tries = 0;
    while (m_world.electronsMinusHoles() < 0 && !m_world.atMaxElectrons())
    {
        if (m_world.parameters().voltageRight >
            m_world.parameters().voltageLeft)
        {
            m_world.electronSourceAgentLeft().tryToInject();
        }
        else
        if (m_world.parameters().voltageLeft >
            m_world.parameters().voltageRight)
        {
            m_world.electronSourceAgentRight().tryToInject();
        }
        else
        {
            if (m_world.randomNumberGenerator().random() > 0.5)
            {
                m_world.electronSourceAgentLeft().tryToInject();
            }
            else
            {
                m_world.electronSourceAgentRight().tryToInject();
            }
        }
        tries += 1;
        if (tries > 1024)
        {
            qFatal("message: can not balance electrons");
        }
    }

    tries = 0;
    while (m_world.holesMinusElectrons() < 0 && !m_world.atMaxHoles())
    {
        if (m_world.parameters().voltageRight >
            m_world.parameters().voltageLeft)
        {
            m_world.holeSourceAgentRight().tryToInject();
        }
        else
        if (m_world.parameters().voltageLeft >
            m_world.parameters().voltageRight)
        {
            m_world.holeSourceAgentLeft().tryToInject();
        }
        else
        {
            if (m_world.randomNumberGenerator().random() > 0.5)
            {
                m_world.holeSourceAgentRight().tryToInject();
            }
            else
            {
                m_world.holeSourceAgentLeft().tryToInject();
            }
        }
        tries += 1;
        if (tries > 1024)
        {
            qFatal("message: can not balance holes");
        }
    }
//    for (unsigned int i = 0;
//         i < m_world.electronDrainAgentRight().successesSinceLast(); i++)
//    {
//        while(!m_world.electronSourceAgentLeft().tryToInject())
//        {
//            m_world.electronSourceAgentLeft().tryToInject();
//        }
//    }

//    for (unsigned int i = 0;
//         i < m_world.electronDrainAgentLeft().successesSinceLast(); i++)
//    {
//        while(!m_world.electronSourceAgentRight().tryToInject())
//        {
//            m_world.electronSourceAgentRight().tryToInject();
//        }
//    }

//    for (unsigned int i = 0;
//         i < m_world.holeDrainAgentRight().successesSinceLast(); i++)
//    {
//        while(!m_world.holeSourceAgentLeft().tryToInject())
//        {
//            m_world.holeSourceAgentLeft().tryToInject();
//        }
//    }

//    for (unsigned int i = 0;
//         i < m_world.holeDrainAgentLeft().successesSinceLast(); i++)
//    {
//        while(!m_world.holeSourceAgentRight().tryToInject())
//        {
//            m_world.holeSourceAgentRight().tryToInject();
//        }
//    }
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
