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
    Simulation::Simulation (SimulationParameters * par, int id) : m_id(id)
    {
        // Careful changing the order of these function calls; you can cause memory problems
        // For example, setPotentialTraps assumes the source and drain have already been created

        // Create the world object
        m_world = new World(par);

        // Place Defects
        this->createDefects();

        // Create Source ( assumes defects were placed because source neighbors can't be defects )
        this->createSource();

        // Create Drain ( assumes defects were placed because drain neighbors can't be defects )
        this->createDrain();

        // Setup Grid Potential
        // Zero potential
        m_world->potential()->setPotentialZero();

        // Add Linear Potential
        if ( m_world->parameters()->potentialLinear ) { m_world->potential()->setPotentialLinear(); }

        // Add Trap Potential ( assumes source and drain were created so that hetero traps don't start growing on the source / drain )
        m_world->potential()->setPotentialTraps();

        // precalculate and store coulomb interaction energies
        m_world->potential()->updateInteractionEnergies();

        // place charges on the grid randomly ( assumes source / drain were created )
        if (m_world->parameters()->gridCharge) seedCharges();

        // Generate grid image
        if (m_world->parameters()->outputGrid)
        {
            m_world->logger()->saveTrapImageToFile(QString("trap-%1-%2-%3.png")
                   .arg(m_world->parameters()->trapPercentage*100.0)
                   .arg(m_world->parameters()->seedPercentage*100.0)
                   .arg(m_id));
        }

        // Output Field Energy
        if (m_world->parameters()->outputFieldPotential)
        {
            m_world->logger()->saveFieldEnergyToFile(QString("field-%1.dat").arg(m_id));
        }

        // Output Defect IDs
        if (m_world->parameters()->outputDefectIDs)
        {
            m_world->logger()->saveDefectIDsToFile(QString("defectIDs-%1.dat").arg(m_id));
        }

        // Output Trap IDs
        if (m_world->parameters()->outputTrapIDs)
        {
            m_world->logger()->saveTrapIDsToFile(QString("trapIDs-%1.dat").arg(m_id));
        }

        // Initialize OpenCL
        m_world->opencl()->initializeOpenCL();
        if ( m_world->parameters()->useOpenCL )
        {
            m_world->opencl()->toggleOpenCL(true);
        }
        else
        {
            m_world->opencl()->toggleOpenCL(false);
        }

        // tick is the global step number of this simulation
        m_tick = 0;
    }

    Simulation::~Simulation ()
    {
        delete m_world;
    }
    void Simulation::createDefects()
    {
        for (int i = 0; i < m_world->grid()->volume (); ++i)
        {
            if (m_world->randomNumberGenerator()->random() < m_world->parameters()->defectPercentage)
            {
                m_world->grid()->setSiteID(i, 1);
                m_world->defectSiteIDs()->push_back(i);
            }
            else
            {
                m_world->grid()->setSiteID(i, 0);
            }
        }
    }

    void Simulation::createDrain()
    {
        m_world->drain()->setSite(m_world->grid()->volume () + 1);
        m_world->grid()->setAgent(m_world->grid()->volume () + 1, m_world->drain());
        m_world->grid()->setSiteID(m_world->grid()->volume () + 1, 3);
        // Now to assign nearest neighbours
        QVector < int >neighbors (0, 0);
        // Loop over layers
        for (int layer = 0; layer < m_world->grid()->depth (); layer++)
        {
            // Get column in this layer that is closest to the source
            QVector < int >column_neighbors_in_layer =
                m_world->grid()->col (m_world->grid()->width () - 1, layer);
            // Loop over this column
            for (int column_site = 0;
                    column_site < column_neighbors_in_layer.size (); column_site++)
            {
                neighbors.push_back (column_neighbors_in_layer[column_site]);
            }
        }
        // Remove defects as possible neighbors
        for ( int i = 0; i < m_world->defectSiteIDs()->size(); i++ )
        {
            int j = m_world->defectSiteIDs()->at(i);
            int k = neighbors.indexOf(j);
            if ( k != -1 ) { neighbors.remove(k); }
        }
        // Set the neighbors of the drain
        m_world->drain()->setNeighbors (neighbors);
    }

    void Simulation::createSource()
    {
        m_world->source()->setSite(m_world->grid()->volume ());
        m_world->grid()->setAgent(m_world->grid()->volume (), m_world->source());
        m_world->grid()->setSiteID(m_world->grid()->volume (), 2);
        // Now to assign nearest neighbours
        QVector < int >neighbors (0, 0);
        // Loop over layers
        for (int layer = 0; layer < m_world->grid()->depth (); layer++)
        {
            // Get column in this layer that is closest to the source
            QVector < int >column_neighbors_in_layer =
                m_world->grid()->col (0, layer);
            // Loop over this column
            for (int column_site = 0;
                    column_site < column_neighbors_in_layer.size (); column_site++)
            {
                neighbors.push_back (column_neighbors_in_layer[column_site]);
            }
        }
        // Remove defects as possible neighbors
        for ( int i = 0; i < m_world->defectSiteIDs()->size(); i++ )
        {
            int j = m_world->defectSiteIDs()->at(i);
            int k = neighbors.indexOf(j);
            if ( k != -1 ) { neighbors.remove(k); }
        }
        // Set the neighbors of the source
        m_world->source()->setNeighbors(neighbors);
        m_world->source()->setMaxCharges( m_world->parameters()->chargePercentage * double (m_world->grid()->volume()) );
    }

    void Simulation::seedCharges ()
    {
        for (int i = 0; i < m_world->source()->maxCharges();)
        {
            // Randomly select a site, ensure it is suitable, if so add a charge agent
            int site = m_world->randomNumberGenerator()->integer(0,m_world->grid()->volume()-1);
            if (m_world->grid()->siteID (site) == 0 && m_world->grid()->agent (site) == 0)
            {
                ChargeAgent *charge = new ChargeAgent (m_world, site);
                m_world->charges()->push_back (charge);
                m_world->source()->incrementCharge ();
                ++i;
            }
        }
    }

    void Simulation::performIterations (int nIterations)
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
                QFuture < void > future = QtConcurrent::map (charges, Simulation::chargeAgentChooseFuture);
                future.waitForFinished();

                // tell the GPU to perform all coulomb calculations
                m_world->opencl()->launchCoulombKernel2();

                //check the answer during debugging
                //m_world->opencl()->compareHostAndDeviceForAllCarriers();

                // now use the results of the coulomb calculations to decide if the carreirs should have moved
                future = QtConcurrent::map (charges, Simulation::chargeAgentDecideFuture);
                future.waitForFinished();
            }
            else // Not using OpenCL
            {
                // Use QtConcurrnet to parallelise the charge calculations
                QFuture < void >future = QtConcurrent::map (charges, Simulation::chargeAgentIterate);

                // We want to wait for it to finish before continuing on
                future.waitForFinished ();
            }

            // Now we are done with the charge movement, move them to the next tick!
            nextTick ();

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

    void Simulation::performInjections (int nInjections)
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

    void Simulation::nextTick ()
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

    inline void Simulation::chargeAgentIterate ( ChargeAgent * chargeAgent)
    {
        // This function performs a single iteration for a charge agent, only thread
        // safe calls can be made in this function. Other threads may access the
        // current state of the chargeAgent, but will not attempt to modify it.
        chargeAgent->transport();
    }

    inline void Simulation::chargeAgentChooseFuture( ChargeAgent * chargeAgent )
    {
        // same rules apply here as for chargeAgentIterate;
        chargeAgent->chooseFuture();
    }

    inline void Simulation::chargeAgentDecideFuture( ChargeAgent * chargeAgent )
    {
        // same rules apply here as for chargeAgentIterate;
        chargeAgent->decideFuture();
    }
}                                // End namespace Langmuir
