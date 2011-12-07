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
        m_world->source()->setNeighbors(neighborsSource());
        m_world->drain()->setNeighbors(neighborsDrain());
    }

    void SolarCell::performIterations(int nIterations)
    {
        //  cout << "Entered performIterations function.\n";
        for (int i = 0; i < nIterations; ++i)
        {
            // Attempt to transport the charges through the film
            QList < ChargeAgent * >&charges = *m_world->charges ();

            // If using OpenCL, launch the Kernel to calculate Coulomb Interactions
            if ( m_world->parameters()->useOpenCL && charges.size() > 0 )
            {
                // first have the charge carriers propose future sites
                //for ( int j = 0; j < charges.size(); j++ ) charges[j]->chooseFuture(j);
                QFuture < void > future = QtConcurrent::map (charges, SolarCell::chargeAgentChooseFuture);
                future.waitForFinished();

                // tell the GPU to perform all coulomb calculations
                m_world->opencl()->launchCoulombKernel2();

                //check the answer during debugging
                //m_world->opencl()->compareHostAndDeviceForAllCarriers();

                // now use the results of the coulomb calculations to decide if the carreirs should have moved
                future = QtConcurrent::map (charges, SolarCell::chargeAgentDecideFuture);
                future.waitForFinished();
            }
            else // Not using OpenCL
            {
                // Use QtConcurrnet to parallelise the charge calculations
                QFuture < void >future = QtConcurrent::map (charges, SolarCell::chargeAgentIterate);

                // We want to wait for it to finish before continuing on
                future.waitForFinished ();
            }

            // Now we are done with the charge movement, move them to the next tick!
            nextTick ();

            // Perform charge injection at the source
            //performInjections (m_world->parameters()->sourceAttempts);

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
        int inject = nInjections;
        if ( inject < 0 )
        {
            inject = m_world->source()->maxCharges() - m_world->charges()->size();
            if ( inject <= 0 )
            {
                return;
            }
        }
        for ( int i = 0; i < inject; i++ )
        {
          int site = m_world->source()->transport();
          if (site != -1)
            {
              ChargeAgent *charge = new ChargeAgent (m_world, site );
              m_world->charges ()->push_back (charge);
            }
        }
    }

    void SolarCell::nextTick()
    {
        // Iterate over all sites to change their state
        QList < ChargeAgent * >&charges = *m_world->charges();
        if( m_world->parameters()->outputStats )
        {
          for (int i = 0; i < charges.size (); ++i)
          {
            charges[i]->completeTick ();
            // Check if the charge was removed - then we should delete it
            if (charges[i]->removed ())
            {
              m_world->logger()->carrierReportLifetimeAndPathlength(i,m_tick);
              delete charges[i];
              charges.removeAt (i);
              m_world->drain()->acceptCharge (-1);
              m_world->source()->decrementCharge ();
              --i;
            }
          }
          m_world->logger()->carrierStreamFlush();
        }
        else
        {
          for (int i = 0; i < charges.size (); ++i)
          {
            charges[i]->completeTick ();
            // Check if the charge was removed - then we should delete it
            if (charges[i]->removed ())
            {
              delete charges[i];
              charges.removeAt (i);
              m_world->drain()->acceptCharge (-1);
              m_world->source()->decrementCharge ();
              --i;
            }
          }
        }
    }

    QVector<int> SolarCell::neighborsSite(int site)
    {
        // Return the indexes of all nearest neighbours
        QVector<int>     nList(0);
        int col = m_world->grid()->getColumn(site);
        int row = m_world->grid()->getRow(site);
        int lay = m_world->grid()->getLayer(site);
        // To the west
        if (col > 0)
          nList.push_back(m_world->grid()->getIndex(col-1,row,lay));
        // To the east
        if (col < m_world->grid()->width() - 1)
          nList.push_back(m_world->grid()->getIndex(col+1,row,lay));
        // To the south
        if (row > 0)
          nList.push_back(m_world->grid()->getIndex(col,row-1,lay));
        // To the north
        if (row < m_world->grid()->height() - 1)
          nList.push_back(m_world->grid()->getIndex(col,row+1,lay));
        // Below
        if (lay > 0)
          nList.push_back(m_world->grid()->getIndex(col,row,lay-1));
        // Above
        if (lay < m_world->grid()->depth() - 1)
          nList.push_back(m_world->grid()->getIndex(col,row,lay+1));
        // Now for the source and the drain....
        //if (col == 0)
        //  nList.push_back(m_world->grid()->getIndex(m_world->grid()->width()-1,m_world->grid()->height()-1,m_world->grid()->depth()-1) + 1);
        //if (col == m_world->grid()->width() - 1)
        //  nList.push_back(m_world->grid()->getIndex(m_world->grid()->width()-1,m_world->grid()->height()-1,m_world->grid()->depth()-1) + 2);
        // Now we have all the nearest neighbours - between 4 and 6 in this case
        return nList;
    }

    inline void SolarCell::chargeAgentIterate ( ChargeAgent * chargeAgent)
    {
        // This function performs a single iteration for a charge agent, only thread
        // safe calls can be made in this function. Other threads may access the
        // current state of the chargeAgent, but will not attempt to modify it.
        chargeAgent->transport();
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
} // End namespace Langmuir
