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
#include <cstdlib>
#include <QtCore/QFuture>
#include <QtCore/QtConcurrentMap>
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

namespace Langmuir
{
  using std::vector;
  using std::cout;
  using std::endl;

    Simulation::Simulation (SimulationParameters * par)
  {
    m_world = new World;        // Create the world object
    m_grid = new CubicGrid (par->gridWidth, par->gridHeight, par->gridDepth);        // Create the grid object
    m_world->setGrid (m_grid);        // Let the world know about the grid
    //createAgents(m_grid->volume(), par->voltageSource, par->voltageDrain, par->defectPercentage); // Create the agents for the simulation
    createAgents (par);

    m_coulombInteraction = par->coulomb;        // Set coulomb interactions
    m_chargedDefects = par->defectsCharged;        // Set charged defects
    m_chargedTraps = par->trapsCharged;        // Set charged traps
    m_zDefect = par->zDefect;        // Set charge on defects
    m_zTrap = par->zTrap;        // Set charge on traps
    m_temperatureKelvin = par->temperatureKelvin;        // Set simulation temperature

    int nCharges = par->chargePercentage * double (m_grid->volume ());        // Calculate the number of charge carriers
      setMaxCharges (nCharges);

    if (par->potentialForm == SimulationParameters::o_linearpotential)        // Set up external potential calculator
      {
        m_potential = new LinearPotential (par->potentialPoints);
        //Add the source to the linear potential
        m_potential->
          addSourcePoint (PotentialPoint (0, 0, 0, par->voltageSource));
        //Add the drain to the linear potential
        m_potential->
          addDrainPoint (PotentialPoint
                         (par->gridWidth, 0, 0, par->voltageDrain));
      }
    else
      {
        qDebug () << "Can not set up the external potential!";
        throw (std::logic_error ("unknown potential"));
      }

    //for ( double layer = 0; layer <= 10.0; layer += 1.0 )
    //{
    // QString outfilename;
    // outfilename.setNum(layer);
    // QFile outputFile(outfilename+".xyz");
    // outputFile.open(QIODevice::WriteOnly | QIODevice::Text);
    // QTextStream out(&outputFile);
    // m_potential->plot(out,0.0,1000.0,0.0,200.0,layer,10.0,10.0);
    //}
    //throw(-1);
    //qDebug() << "setting hetero traps " << par->trapsHetero;
    if (par->trapsHetero)
      heteroTraps (par);
    else
      updatePotentials (par);

    updateInteractionEnergies ();        // precalculate potentials for interacting charges ( carriers, defects, traps )
    if (par->gridCharge)
      seedCharges ();                // Charge the grid up is specified

  }

  Simulation::~Simulation ()
  {
    delete m_world;
    delete m_grid;
    delete m_potential;
    m_world = 0;
    m_grid = 0;
  }

  void Simulation::setMaxCharges (int n)
  {
    m_source->setMaxCharges (n);
    m_maxcharges = n;
  }

  bool Simulation::seedCharges ()
  {
    // We randomly place all of the charges in the grid
    Grid *grid = m_world->grid ();

    unsigned int nSites = grid->width () * grid->height () * grid->depth ();
    int maxCharges = m_source->maxCharges ();

    for (int i = 0; i < maxCharges;)
      {
        // Randomly select a site, ensure it is suitable, if so add a charge agent
        unsigned int site =
          int (m_world->random () * (double (nSites) - 0.00001));
        if (grid->siteID (site) == 0 && grid->agent (site) == 0)
          {
            ChargeAgent *charge =
              new ChargeAgent (m_world, site, m_coulombInteraction,
                               m_chargedDefects, m_chargedTraps,
                               m_temperatureKelvin, m_zDefect, m_zTrap);
            m_world->charges ()->push_back (charge);
            m_source->incrementCharge ();
            ++i;
          }
      }
    return true;
  }

  void Simulation::setCoulombInteractions (bool enabled)
  {
    m_coulombInteraction = enabled;
  }

  void Simulation::setChargedDefects (bool on)
  {
    m_chargedDefects = on;
  }

  void Simulation::setChargedTraps (bool ok)
  {
    m_chargedTraps = ok;
  }

  void Simulation::setZdefect (int zDefect)
  {
    m_zDefect = zDefect;
  }

  void Simulation::setZtrap (int zTrap)
  {
    m_zTrap = zTrap;
  }

  void Simulation::setTemperature (double temperatureKelvin)
  {
    m_temperatureKelvin = temperatureKelvin;
  }

  void Simulation::performIterations (int nIterations)
  {
    //  cout << "Entered performIterations function.\n";
    for (int i = 0; i < nIterations; ++i)
      {
        // Attempt to transport the charges through the film
        QList < ChargeAgent * >&charges = *m_world->charges ();

        // Use QtConcurrnet to parallelise the charge calculations
        QFuture < void >future =
          QtConcurrent::map (charges, Simulation::chargeAgentIterate);

        // We want to wait for it to finish before continuing on
        future.waitForFinished ();

        // Now we are done with the charge movement, move them to the next tick!
        nextTick ();

        // Begin by performing charge injection at the source
        unsigned int site = m_source->transport ();
        //qDebug () << "Source transport returned site:" << site;

        if (site != errorValue)
          {
            ChargeAgent *charge =
              new ChargeAgent (m_world, site, m_coulombInteraction,
                               m_chargedDefects, m_chargedTraps,
                               m_temperatureKelvin, m_zDefect, m_zTrap);
            m_world->charges ()->push_back (charge);
          }
      }
  }

  void Simulation::nextTick ()
  {
    // Iterate over all sites to change their state
    QList < ChargeAgent * >&charges = *m_world->charges ();
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

  void Simulation::printGrid ()
  {
    system ("clear");
    unsigned int width = m_world->grid ()->width ();
    unsigned int height = m_world->grid ()->height ();
    for (unsigned int j = 0; j < height; ++j)
      {
        qDebug () << "||";
        for (unsigned int i = 0; i < width; ++i)
          {
            if (m_world->grid ()->agent (i + j * width))
              {                        // Charge is present
                if (m_world->grid ()->siteID (i + j * width) == 0)
                  qDebug () << "*";
                else
                  qDebug () << "x";
              }
            else if (m_world->grid ()->siteID (i + j * width) == 1)
              qDebug () << "O";
            else
              qDebug () << " ";
          }
        qDebug () << "||\n";
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

//  void Simulation::createAgents( unsigned int numAgents, double sourcePotential, double drainPotential, double defectPercent )
  void Simulation::createAgents (SimulationParameters * par)
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
    for (unsigned int i = 0; i < m_grid->volume (); ++i)
      {
        if (m_world->random () < par->defectPercentage)
          {
            m_world->grid ()->setSiteID (i, 1);        //defect
            m_world->chargedDefects ()->push_back (i);        //Add defect to list - for charged defects
          }
        else
          {
            m_world->grid ()->setSiteID (i, 0);        //Charge carrier site
          }
      }
    // Add the source and the drain
    m_source =
      new SourceAgent (m_world, m_grid->volume (), par->voltageSource, par->sourceBarrier);
    m_world->grid ()->setAgent (m_grid->volume (), m_source);
    m_world->grid ()->setSiteID (m_grid->volume (), 2);
    m_drain =
      new DrainAgent (m_world, m_grid->volume () + 1, par->voltageDrain);
    m_world->grid ()->setAgent (m_grid->volume () + 1, m_drain);
    m_world->grid ()->setSiteID (m_grid->volume () + 1, 3);

    // Now to assign nearest neighbours for the electrodes.
    vector < unsigned int >neighbors (0, 0);

    // Loop over layers
    for (unsigned int layer = 0; layer < m_grid->depth (); layer++)
      {
        // Get column in this layer that is closest to the source
        vector < unsigned int >column_neighbors_in_layer =
          m_grid->col (0, layer);
        // Loop over this column
        for (unsigned int column_site = 0;
             column_site < column_neighbors_in_layer.size (); column_site++)
          {
            neighbors.push_back (column_neighbors_in_layer[column_site]);
          }
      }
    // Set the neighbors of the source
    m_source->setNeighbors (neighbors);
    neighbors.clear ();

    // Loop over layers
    for (unsigned int layer = 0; layer < m_grid->depth (); layer++)
      {
        // Get column in this layer that is closest to the source
        vector < unsigned int >column_neighbors_in_layer =
          m_grid->col (m_grid->width () - 1, layer);
        // Loop over this column
        for (unsigned int column_site = 0;
             column_site < column_neighbors_in_layer.size (); column_site++)
          {
            neighbors.push_back (column_neighbors_in_layer[column_site]);
          }
      }
    // Set the neighbors of the drain
    m_drain->setNeighbors (neighbors);
    neighbors.clear ();
  }

  void Simulation::destroyAgents ()
  {
    delete m_source;
    m_source = 0;
    delete m_drain;
    m_drain = 0;
  }
  void Simulation::heteroTraps (SimulationParameters * par)
  {
    double tPotential = 0;
    double gaussianDisorder = 0.0;

    //qDebug() << "entering heteroTraps" << par->trapPercentage;

    for (unsigned int x = 0; x < m_grid->width (); x++)
      {
        for (unsigned int y = 0; y < m_grid->height (); y++)
          {
            for (unsigned int z = 0; z < m_grid->depth (); z++)
              {
                // Get the potential at this site
                tPotential =
                  m_potential->calculate (x + 0.5, y + 0.5, z + 0.5);

                // Randomly add some noise
                if (par->gaussianNoise)
                  {
                    gaussianDisorder =
                      m_world->random (par->gaussianAVERG,
                                       par->gaussianSTDEV);
                    tPotential += gaussianDisorder;
                  }

                // The site ID ( this is bad its specific to 3D cubic grids )
                unsigned int site =
                  x + m_grid->width () * y + z * m_grid->area ();

                // Add seeds for heterogeneous traps
                if (m_world->random () <
                    ((par->seedPercentage) * (par->trapPercentage)))
                  {
                    m_grid->setPotential (site,
                                          (tPotential + par->deltaEpsilon));
                    m_world->chargedTraps ()->push_back (site);
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
      (par->trapPercentage) * (par->gridWidth) * (par->gridHeight) *
      (par->gridDepth);

    //qDebug() << "Trap Seeds: " << m_world->chargedTraps()->size(); 
    //for (int i = 0; i < m_world->chargedTraps()->size(); i++)
    //{
    //      qDebug() << (*m_world->chargedTraps())[i]; 
    //}

    //Now loop over our seeds
    while ((m_world->chargedTraps ()->size ()) < trapCount)
      {
        // Select a random trap
        unsigned int trapSeed =
          int (m_world->random () *
               (m_world->chargedTraps ()->size () - 1.0e-20));

        // Select a random neighbor
        unsigned int newTrap;
        vector < unsigned int >m_neighbors = m_grid->neighbors (trapSeed);
        //qDebug() << "Neighbors: " << m_neighbors.size();
        newTrap =
          m_neighbors[int
                      (m_world->random () * (m_neighbors.size () - 1.0e-20))];
        //qDebug() << "newTrap: " << newTrap; 
        // Make sure it is not already a trap site

        if (m_world->chargedTraps ()->contains (newTrap))
          continue;

        else
          {                        // create a new trap site and save it
            m_grid->setPotential (newTrap, (tPotential + par->deltaEpsilon));
            m_world->chargedTraps ()->push_back (newTrap);
            //qDebug() << "Traps: " << m_world->chargedTraps()->size();
          }
      }
    qDebug () << "Traps: " << m_world->chargedTraps ()->size ();

    //Set the potential of the Source
    m_grid->setSourcePotential (m_potential->calculate (0.0, 0.0, 0.0));

    //Set the potential of the Drain
    m_grid->setDrainPotential (m_potential->
                               calculate (double (m_grid->width ()), 0.0,
                                          0.0));

  }

  void Simulation::updatePotentials (SimulationParameters * par)
  {

    double tPotential = 0;
    double gaussianDisorder = 0.0;

    for (unsigned int x = 0; x < m_grid->width (); x++)
      {
        for (unsigned int y = 0; y < m_grid->height (); y++)
          {
            for (unsigned int z = 0; z < m_grid->depth (); z++)
              {

                // Get the potential at this site
                tPotential =
                  m_potential->calculate (x + 0.5, y + 0.5, z + 0.5);

                // Randomly add some noise
                if (par->gaussianNoise)
                  {
                    gaussianDisorder =
                      m_world->random (par->gaussianAVERG,
                                       par->gaussianSTDEV);
                    tPotential += gaussianDisorder;
                  }

                // The site ID ( this is bad its specific to 3D cubic grids )
                unsigned int site =
                  x + m_grid->width () * y + z * m_grid->area ();

                // We can randomly tweak a site energy
                if (m_world->random () < par->trapPercentage)
                  {
                    m_grid->setPotential (site,
                                          (tPotential + par->deltaEpsilon));
                    m_world->chargedTraps ()->push_back (site);
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
    m_grid->setDrainPotential (m_potential->
                               calculate (double (m_grid->width ()), 0.0,
                                          0.0));

/*
   // We are assuming one source, one drain that are parallel.
   // The most efficient way to calculate this is to work out the potential
   // for each column in our system and then assign it to each agent.
   int width = m_grid->width();

   double m = (m_drain->potential() - m_source->potential()) / double(width);
   double c = m_source->potential();

   // The source to drain distance is simply the width of the device.
   for (int i = 0; i < width; i++) {
    // The potential of this column
    double tPotential = m * (double(i)+0.5) + c;
    // Loop over layers and assign the potential to this column in layer
    for ( unsigned int layer = 0; layer < m_grid->depth(); layer++ ) 
    {
     // All the members of this column in the given layer
     vector<unsigned int> column_members = m_grid->col(i,layer);
     // Loop over the members of this column in the given layer
     for (vector<unsigned int>::iterator j = column_members.begin(); j != column_members.end(); j++)
     {
      // We can randomly tweak a site energy
      if (m_world->random() < trapPercent)
      {
       m_grid->setPotential(*j, (tPotential + deltaEpsilon)); 
       m_world->chargedTraps()->push_back(*j);
      }
      // Assign the potential
      else { m_grid->setPotential(*j, tPotential); }
     }
    }
    }
    // Set the potential of the drain site
    //tPotential = m * (double(width)+0.5) + c;        
    //m_grid->setPotential(width*m_grid->height()*m_grid->depth()+1, tPotential);
*/
  }

  void Simulation::updateInteractionEnergies ()
  {
    //These values are used in Coulomb interaction calculations.
    TripleIndexArray & energies = m_world->interactionEnergies ();

    // Currently hard coding a cut off of 50nm - no interactions beyond that added
    unsigned int cutoff = 50;

    energies.resize (cutoff, cutoff, cutoff);
    const double q = 1.60217646e-19;        // Magnitude of charge on an electron

    // Now calculate the numbers we need
    for (unsigned int col = 0; col < cutoff; ++col)
      {
        for (unsigned int row = 0; row < cutoff; ++row)
          {
            for (unsigned int lay = 0; lay < cutoff; ++lay)
              {
                energies (col, row, lay) =
                  q / sqrt (col * col + row * row + lay * lay);
              }
          }
      }

    // The interaction energy is zero when R is zero
    for (unsigned int col = 0; col < cutoff; ++col)
      energies (col, col, col) = 0;

  }

  inline void Simulation::chargeAgentIterate (ChargeAgent * chargeAgent)
  {
    // This function performs a single iteration for a charge agent, only thread
    // safe calls can be made in this function. Other threads may access the
    // current state of the chargeAgent, but will not attempt to modify it.
    chargeAgent->transport ();
  }

}                                // End namespace Langmuir
