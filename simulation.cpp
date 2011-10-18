#include "simulation.h"
#include "rand.h"
#include "world.h"
#include "linearpotential.h"
#include "cubicgrid.h"
#include "chargeagent.h"
#include "sourceagent.h"
#include "drainagent.h"
#include "inputparser.h"
#include <iostream>
#include <QtCore/QFuture>
#include <QtCore/QtConcurrentMap>
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QPrinter>
#include <QPainter>
#include <QColor>

namespace Langmuir
{
    Simulation::Simulation (SimulationParameters * par, int id) : m_id(id)
    {
        // Store the address of the simulation parameters object
        m_parameters = par;

        // Create the world object
        m_world = new World( par->randomSeed );

        // Create the grid object
        m_grid = new CubicGrid (m_parameters->gridWidth, m_parameters->gridHeight, m_parameters->gridDepth);

        // Let the world know about the grid
        m_world->setGrid (m_grid);

        // Let the world know about the simulation parameters
        m_world->setParameters (m_parameters);

        // Create the agents
        createAgents ();

        // Calculate the number of charge carriers
        int nCharges = m_parameters->chargePercentage * double (m_grid->volume ());
        m_source->setMaxCharges (nCharges);
        m_maxcharges = nCharges;

        // Set up and Calculate potential due to electrodes
        if (m_parameters->potentialForm == 0)
        {
            m_potential = new LinearPotential (m_parameters->potentialPoints);
            //Add the source to the linear potential
            m_potential->
            addSourcePoint (PotentialPoint (0, 0, 0, m_parameters->voltageSource));
            //Add the drain to the linear potential
            m_potential->
            addDrainPoint (PotentialPoint
                           (m_parameters->gridWidth, 0, 0, m_parameters->voltageDrain));
        }
        else
        {
            qDebug () << "Can not set up the external potential!";
            throw (std::logic_error ("unknown potential"));
        }

        // precalculate and store site energies
        if (m_parameters->trapsHeterogeneous) heteroTraps ();
        else updatePotentials ();

        // precalculate and store coulomb interaction energies
        updateInteractionEnergies ();

        // place charges on the grid randomly
        if (m_parameters->gridCharge) seedCharges ();

        // Generate grid image
        if (m_parameters->outputGrid) gridImage();

        // Output Field Energy
        if (m_parameters->outputFieldPotential) m_world->saveFieldEnergyToFile(QString("field-%1.dat").arg(m_id));

        // Output Traps Energy
        if (m_parameters->outputTrapsPotential) m_world->saveTrapEnergyToFile(QString("trap-%1.dat").arg(m_id));

        // Output Defect IDs
        if (m_parameters->outputDefectIDs) m_world->saveDefectIDsToFile(QString("defectIDs-%1.dat").arg(m_id));

        // Output Trap IDs
        if (m_parameters->outputTrapIDs) m_world->saveTrapIDsToFile(QString("trapIDs-%1.dat").arg(m_id));

        // Initialize OpenCL
        m_world->initializeOpenCL();
        if ( m_parameters->useOpenCL )
        {
            m_world->toggleOpenCL(true);
        }
        else
        {
            m_world->toggleOpenCL(false);
        }

        // tick is the global step number of this simulation
        m_tick = 0;
    }

    Simulation::~Simulation ()
    {
        delete m_world;
        delete m_grid;
        delete m_potential;
        m_world = 0;
        m_grid = 0;
    }

    bool Simulation::seedCharges ()
    {
        // We randomly place all of the charges in the grid
        Grid *grid = m_world->grid ();

        int nSites = grid->width () * grid->height () * grid->depth ();
        int maxCharges = m_source->maxCharges ();

        for (int i = 0; i < maxCharges;)
        {
            // Randomly select a site, ensure it is suitable, if so add a charge agent
            int site =
                int (m_world->random () * (double (nSites) - 1.0e-20));
            if (grid->siteID (site) == 0 && grid->agent (site) == 0)
            {
                ChargeAgent *charge = new ChargeAgent (m_world, site);
                m_world->charges ()->push_back (charge);
                m_source->incrementCharge ();
                ++i;
            }
        }
        return true;
    }

    void Simulation::performIterations (int nIterations)
    {
        //  cout << "Entered performIterations function.\n";
        for (int i = 0; i < nIterations; ++i)
        {
            // Attempt to transport the charges through the film
            QList < ChargeAgent * >&charges = *m_world->charges ();

            // If using OpenCL, launch the Kernel to calculate Coulomb Interactions
            if ( m_parameters->useOpenCL && charges.size() > 0 )
            {
                // first have the charge carriers propose future sites
                //for ( int j = 0; j < charges.size(); j++ ) charges[j]->chooseFuture(j);
                QFuture < void > future = QtConcurrent::map (charges, Simulation::chargeAgentChooseFuture);
                future.waitForFinished();

                // tell the GPU to perform all coulomb calculations
                m_world->launchCoulombKernel2();

                //check the answer during debugging
                //m_world->compareHostAndDeviceForAllCarriers();

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
            performInjections (m_parameters->sourceAttempts);

            m_tick += 1;
        }
        // Output Coulomb Energy
        if (m_parameters->outputCoulombPotential)
        {
            m_world->launchCoulombKernel1();
            m_world->saveCoulombEnergyToFile( QString("coulomb-%1-%2.dat").arg(m_id).arg(m_tick) );
        }
        // Output Carrier Positions and IDs
        if (m_parameters->outputCarriers)
        {
            m_world->saveCarrierIDsToFile( QString("carriers-%1-%2.dat").arg(m_id).arg(m_tick) );
        }
    }

    void Simulation::performInjections (int nInjections)
    {
        int inject = nInjections;
        if ( inject < 0 )
        {
            inject = m_maxcharges - this->charges();
            if ( inject <= 0 )
            {
                return;
            }
        }
        for ( int i = 0; i < inject; i++ )
        {
          int site = m_source->transport();
          if (site != errorValue)
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
        if( m_parameters->outputStats )
        {
          for (int i = 0; i < charges.size (); ++i)
          {
            charges[i]->completeTick ();
            // Check if the charge was removed - then we should delete it
            if (charges[i]->removed ())
            {
              m_world->statMessage( QString( "%1 %2 %3\n" )
                                       .arg(m_tick,m_parameters->outputWidth)
                                       .arg(charges[i]->lifetime(),m_parameters->outputWidth)
                                       .arg(charges[i]->distanceTraveled(),m_parameters->outputWidth) );
              delete charges[i];
              charges.removeAt (i);
              m_drain->acceptCharge (-1);
              m_source->decrementCharge ();
              --i;
            }
          }
          m_world->statFlush();
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
              m_drain->acceptCharge (-1);
              m_source->decrementCharge ();
              --i;
            }
          }
        }
    }

    void Simulation::printGrid ()
    {
        system ("clear");
        int width = m_world->grid ()->width ();
        int height = m_world->grid ()->height ();
        for (int j = 0; j < height; ++j)
        {
            std::cout << "||";
            for (int i = 0; i < width; ++i)
            {
                int index = m_world->grid()->getIndex(i,j,0);
                if (m_world->grid ()->agent (index))
                {
                    if (m_world->grid ()->siteID(index) == 0)
                    {
                        std::cout << "*";
                    }
                    else
                    {
                        std::cout << m_world->grid()->siteID(index);
                    }
                }
                else std::cout << " ";
            }
            std::cout << "||\n";
        }
    }

    unsigned long Simulation::totalChargesAccepted ()
    {
        return m_drain->acceptedCharges ();
    }

    unsigned long Simulation::charges ()
    {
        return m_world->charges ()->size ();
    }

    void Simulation::createAgents ()
    {
        /**
         * Each site is assigned an ID, this ID identifies the type of site and
         * coupling constants can also be retrieved from the world::coupling()
         * matrix. The sites are as follows,
         *
         * 0: Normal transport site.
         * 1: Defect site.
         * 2: Source site.
         * 3: Drain site.
         *
         * Currently a 4x4 matrix stores coupling. If a charge is transported to a
         * drain site it is removed from the system.
         */

        // Add the agents
        for (int i = 0; i < m_grid->volume (); ++i)
        {
            if (m_world->random () < m_parameters->defectPercentage)
            {
                m_world->grid ()->setSiteID (i, 1);        //defect
                m_world->defectSiteIDs ()->push_back (i);        //Add defect to list - for charged defects
            }
            else
            {
                m_world->grid ()->setSiteID (i, 0);        //Charge carrier site
            }
        }
        // Add the source and the drain
        m_source = new SourceAgent (m_world, m_grid->volume ());
        m_world->grid ()->setAgent (m_grid->volume (), m_source);
        m_world->grid ()->setSiteID (m_grid->volume (), 2);
        m_drain = new DrainAgent (m_world, m_grid->volume () + 1);
        m_world->grid ()->setAgent (m_grid->volume () + 1, m_drain);
        m_world->grid ()->setSiteID (m_grid->volume () + 1, 3);

        // Now to assign nearest neighbours for the electrodes.
        QVector < int >neighbors (0, 0);

        switch ( m_parameters->hoppingRange )
        {

            default:
            {
                // Loop over layers
                for (int layer = 0; layer < m_grid->depth (); layer++)
                {
                    // Get column in this layer that is closest to the source
                    QVector < int >column_neighbors_in_layer =
                        m_grid->col (0, layer);
                    // Loop over this column
                    for (int column_site = 0;
                            column_site < column_neighbors_in_layer.size (); column_site++)
                    {
                        neighbors.push_back (column_neighbors_in_layer[column_site]);
                    }
                }
                // Set the neighbors of the source
                m_source->setNeighbors (neighbors);
                neighbors.clear ();

                // Loop over layers
                for (int layer = 0; layer < m_grid->depth (); layer++)
                {
                    // Get column in this layer that is closest to the source
                    QVector < int >column_neighbors_in_layer =
                        m_grid->col (m_grid->width () - 1, layer);
                    // Loop over this column
                    for (int column_site = 0;
                            column_site < column_neighbors_in_layer.size (); column_site++)
                    {
                        neighbors.push_back (column_neighbors_in_layer[column_site]);
                    }
                }
                // Set the neighbors of the drain
                m_drain->setNeighbors (neighbors);
                neighbors.clear ();
                break;
            }

        }

    }

    void Simulation::destroyAgents ()
    {
        delete m_source;
        m_source = 0;
        delete m_drain;
        m_drain = 0;
    }

    void Simulation::heteroTraps ()
    {
        double tPotential = 0;
        double gaussianDisorder = 0.0;

        //qDebug() << "entering heteroTraps" << par->trapPercentage;

        for (int x = 0; x < m_grid->width (); x++)
        {
            for (int y = 0; y < m_grid->height (); y++)
            {
                for (int z = 0; z < m_grid->depth (); z++)
                {
                    // Get the potential at this site
                    tPotential =
                        m_potential->calculate (x + 0.5, y + 0.5, z + 0.5);

                    // Randomly add some noise
                    if (m_parameters->gaussianNoise)
                    {
                        gaussianDisorder =
                            m_world->random (m_parameters->gaussianAverg,
                                             m_parameters->gaussianStdev);
                        tPotential += gaussianDisorder;
                    }

                    // The site ID ( this is bad its specific to 3D cubic grids )
                    int site =
                        x + m_grid->width () * y + z * m_grid->area ();

                    // Add seeds for heterogeneous traps
                    if (m_world->random () <
                            ((m_parameters->seedPercentage) * (m_parameters->trapPercentage)))
                    {
                        m_grid->setPotential (site,
                                              (tPotential + m_parameters->deltaEpsilon));
                        m_world->trapSiteIDs ()->push_back (site);
                    }

                    // Assign the potential
                    else
                    {
                        m_grid->setPotential (site, tPotential);
                    }
                }
            }
        }

        int trapCount =
            (m_parameters->trapPercentage) * (m_parameters->gridWidth) * (m_parameters->gridHeight) *
            (m_parameters->gridDepth);

        //qDebug() << "Trap Seeds: " << m_world->chargedTraps()->size();
        //for (int i = 0; i < m_world->chargedTraps()->size(); i++)
        //{
        //      qDebug() << (*m_world->chargedTraps())[i];
        //}

        //Now loop over our seeds
        while ((m_world->trapSiteIDs ()->size ()) < trapCount)
        {
            // Select a random trap
            int trapSeed =
                int (m_world->random () *
                     (m_world->trapSiteIDs ()->size () - 1.0e-20));

            // Select a random neighbor
            int newTrap;
            QVector < int >m_neighbors = m_grid->neighbors(m_world->trapSiteIDs()->at(trapSeed),1);
            //qDebug() << "Neighbors: " << m_neighbors.size();
            newTrap =
                m_neighbors[int
                            (m_world->random () * (m_neighbors.size () - 1.0e-20))];

            // Check to make sure the newTrap isn't the source or drain...
            if ((m_world->grid()->siteID(newTrap)) == 2 ||
                    (m_world->grid()->siteID(newTrap)) == 3)
                continue;

            //qDebug() << "newTrap: " << newTrap;
            // Make sure it is not already a trap site

            if (m_world->trapSiteIDs ()->contains (newTrap))
                continue;

            else
            {
                // create a new trap site and save it
                int x = m_grid->getColumn(newTrap);
                int y = m_grid->getRow(newTrap);
                int z = m_grid->getLayer(newTrap);
                tPotential = m_potential->calculate (x + 0.5, y + 0.5, z + 0.5);
                m_grid->setPotential (newTrap, (tPotential + m_parameters->deltaEpsilon));
                m_world->trapSiteIDs ()->push_back (newTrap);
                //qDebug() << "Traps: " << m_world->chargedTraps()->size();
            }
        }
        //qDebug () << "Traps: " << m_world->chargedTraps ()->size ();

        //for (int i = 0; i < m_grid->volume (); i++)
        //{
        //  qDebug() << i << "  " << m_grid->potential(i);
        //}


        //Set the potential of the Source
        m_grid->setSourcePotential (m_potential->calculate (0.0, 0.0, 0.0));

        //qDebug() << "Source Potential: " << m_grid->potential(m_grid->volume());

        //Set the potential of the Drain
        m_grid->setDrainPotential (m_potential->
                                   calculate (double (m_grid->width ()), 0.0,
                                              0.0));
        //qDebug() << "Drain Potential: " << m_grid->potential(m_grid->volume() + 1);
    }

    void Simulation::gridImage()
    {
        {
            QPrinter printer;
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOrientation(QPrinter::Landscape);
            printer.setOutputFileName("grid" + QString::number(m_parameters->trapPercentage * 100) + ".pdf");

            QPainter painter;
            if (! painter.begin(&printer)) //Failed to open the file
            {

                qWarning("failed to open file, is it writable?");
                return ;
            }

            painter.setWindow(QRect(0,0,1024,1024));
            painter.setBrush(Qt::blue);
            painter.setPen(Qt::darkBlue);

            for(int i = 0; i < m_world->trapSiteIDs()->size(); i++)
            {
                int rowCoord = m_grid->getRow(m_world->trapSiteIDs()->at(i));
                //qDebug() << rowCoord;
                int colCoord = m_grid->getColumn(m_world->trapSiteIDs()->at(i));
                //qDebug() << colCoord;
                painter.drawRect(colCoord,rowCoord,1,1);
            }
            painter.end();
        }
    }

    void Simulation::updatePotentials ()
    {
        double tPotential = 0;
        double gaussianDisorder = 0.0;

        for (int x = 0; x < m_grid->width (); x++)
        {
            for (int y = 0; y < m_grid->height (); y++)
            {
                for (int z = 0; z < m_grid->depth (); z++)
                {

                    // Get the potential at this site
                    tPotential =
                        m_potential->calculate (x + 0.5, y + 0.5, z + 0.5);

                    // Randomly add some noise
                    if (m_parameters->gaussianNoise)
                    {
                        gaussianDisorder =
                            m_world->random (m_parameters->gaussianAverg,
                                             m_parameters->gaussianStdev);
                        tPotential += gaussianDisorder;
                    }

                    // The site ID ( this is bad its specific to 3D cubic grids )
                    int site =
                        x + m_grid->width () * y + z * m_grid->area ();

                    // We can randomly tweak a site energy
                    if (m_world->random () < m_parameters->trapPercentage)
                    {
                        m_grid->setPotential (site,
                                              (tPotential + m_parameters->deltaEpsilon));
                        m_world->trapSiteIDs ()->push_back (site);
                    }

                    // Assign the potential
                    else
                    {
                        m_grid->setPotential (site, tPotential);
                    }

                }
            }
        }

        //Set the potential of the Source
        m_grid->setSourcePotential (m_potential->calculate (0.0, 0.0, 0.0));

        //Set the potential of the Drain
        m_grid->setDrainPotential (m_potential->calculate (double (m_grid->width ()), 0.0,0.0));
    }

    void Simulation::updateInteractionEnergies ()
    {
        //These values are used in Coulomb interaction calculations.
        TripleIndexArray & energies = m_world->interactionEnergies ();

        energies.resize (m_parameters->electrostaticCutoff, m_parameters->electrostaticCutoff, m_parameters->electrostaticCutoff);

        // Now calculate the numbers we need
        for (int col = 0; col < m_parameters->electrostaticCutoff; ++col)
        {
            for (int row = 0; row < m_parameters->electrostaticCutoff; ++row)
            {
                for (int lay = 0; lay < m_parameters->electrostaticCutoff; ++lay)
                {
                    double r = sqrt (col * col + row * row + lay * lay);
                    if ( r > 0 && r < m_parameters->electrostaticCutoff )
                    {
                        energies (col, row, lay) = m_parameters->elementaryCharge / r;
                    }
                    else
                    {
                        energies (col, row, lay) = 0.0;
                    }
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
