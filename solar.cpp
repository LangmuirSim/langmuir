#include "solar.h"
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
    SolarCell::SolarCell( SimulationParameters * par, int id ) : Simulation(par,id)
    {
    }

    void SolarCell::performIterations(int nIterations)
    {
        //m_world->electrons()->at(0)->coulombInteraction( m_world->electrons()->at(0)->site(true) );
        //  cout << "Entered performIterations function.\n";
        for (int i = 0; i < nIterations; ++i)
        {
            // Attempt to transport the charges through the film
            QList < ChargeAgent * >&electrons = *m_world->electrons ();
            QList < ChargeAgent * >&holes = *m_world->holes ();

            // If using OpenCL, launch the Kernel to calculate Coulomb Interactions
            if ( m_world->parameters()->useOpenCL && ( electrons.size() > 0 || holes.size() > 0 ) )
            {
                // first have the charge carriers propose future sites
                //for ( int j = 0; j < charges.size(); j++ ) charges[j]->chooseFuture(j);
                QFuture < void > future1 = QtConcurrent::map (electrons, SolarCell::chargeAgentChooseFuture);
                QFuture < void > future2 = QtConcurrent::map (holes, SolarCell::chargeAgentChooseFuture);
                future1.waitForFinished();
                future2.waitForFinished();

                // tell the GPU to perform all coulomb calculations
                m_world->opencl()->launchCoulombKernel2();

                //check the answer during debugging
                //m_world->opencl()->compareHostAndDeviceForAllCarriers();
                //qFatal("done");

                // now use the results of the coulomb calculations to decide if the carreirs should have moved
                future1 = QtConcurrent::map (electrons, SolarCell::chargeAgentDecideFuture);
                future2 = QtConcurrent::map (holes, SolarCell::chargeAgentDecideFuture);
                future1.waitForFinished();
                future2.waitForFinished();
            }
            else // Not using OpenCL
            {
                // Use QtConcurrnet to parallelise the charge calculations
                QFuture < void > future1 = QtConcurrent::map (electrons, SolarCell::chargeAgentIterate);
                QFuture < void > future2 = QtConcurrent::map (holes, SolarCell::chargeAgentIterate);

                // We want to wait for it to finish before continuing on
                future1.waitForFinished();
                future2.waitForFinished();
            }

            // Now we are done with the charge movement, move them to the next tick!
            nextTick();

            // Perform charge injection at the source
            performInjections (m_world->parameters()->sourceAttempts);

            m_tick += 1;
        }
        // Output Coulomb Energy
        if (m_world->parameters()->outputCoulombPotential)
        {
            m_world->opencl()->launchCoulombKernel1();
            m_world->logger()->saveCoulombEnergyToFile( QString("coulomb-%1-%2.dat").arg(m_id).arg(m_tick) );
        }
        // Output Carrier Positions and IDs
        if (m_world->parameters()->outputCarriers)
        {
            m_world->logger()->saveCarrierIDsToFile( QString("carriers-%1-%2.dat").arg(m_id).arg(m_tick) );
        }
    }

    void SolarCell::performInjections(int nInjections)
    {
        for ( int i = 0; i < m_world->sources()->size(); i++ )
        {
            m_world->sources()->at(i)->tryToInject();
        }
        /*
        int inject = nInjections;
        if ( inject < 0 )
        {
            inject = m_world->sourceL()->maxCharges() - m_world->electrons()->size();
            if ( inject <= 0 )
            {
                return;
            }
        }
        for ( int i = 0; i < inject; i++ )
        {
          int site = m_world->sourceL()->transport();
          if (site != -1)
            {
              ChargeAgent *charge = new ChargeAgent (Agent::Hole,m_world, site );
              m_world->electrons ()->push_back (charge);
            }
        }
        */
    }

    void SolarCell::nextTick()
    {
        // Iterate over all sites to change their state
        QList < ChargeAgent * >&electrons = *m_world->electrons();
        QList < ChargeAgent * >&holes = *m_world->holes();
        if (m_world->parameters()->outputStats)
        {
            for (int i = 0; i < electrons.size (); ++i)
            {
              electrons[i]->completeTick();
              // Check if the charge was removed - then we should delete it
              if (electrons[i]->removed())
              {
                m_world->logger()->carrierReportLifetimeAndPathlength(i,m_tick);
                delete electrons[i];
                electrons.removeAt (i);
                --i;
              }
            }
            for (int i = 0; i < holes.size (); ++i)
            {
              holes[i]->completeTick ();
              // Check if the charge was removed - then we should delete it
              if (holes[i]->removed())
              {
                m_world->logger()->carrierReportLifetimeAndPathlength(i,m_tick);
                delete holes[i];
                holes.removeAt (i);
                --i;
              }
            }
            m_world->logger()->carrierStreamFlush();
        }
        else
        {
            for (int i = 0; i < electrons.size (); ++i)
            {
              electrons[i]->completeTick();
              // Check if the charge was removed - then we should delete it
              if (electrons[i]->removed ())
              {
                delete electrons[i];
                electrons.removeAt (i);
                --i;
              }
            }
            for (int i = 0; i < holes.size (); ++i)
            {
              holes[i]->completeTick ();
              // Check if the charge was removed - then we should delete it
              if (holes[i]->removed ())
              {
                delete holes[i];
                holes.removeAt (i);
                --i;
              }
            }
        }
    }

    inline void SolarCell::chargeAgentIterate ( ChargeAgent * chargeAgent)
    {
        // This function performs a single iteration for a charge agent, only thread
        // safe calls can be made in this function. Other threads may access the
        // current state of the chargeAgent, but will not attempt to modify it.
        chargeAgent->chooseFuture();
        chargeAgent->decideFuture();
    }

    inline void SolarCell::chargeAgentChooseFuture( ChargeAgent * chargeAgent )
    {
        // same rules apply here as for chargeAgentIterate;
        chargeAgent->chooseFuture();
    }

    inline void SolarCell::chargeAgentDecideFuture( ChargeAgent * chargeAgent )
    {
        // same rules apply here as for chargeAgentIterate;
        chargeAgent->decideFuture();
    }
}
