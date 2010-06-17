#include "simulation.h"
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

namespace Langmuir
{
  using std::vector;
  using std::cout;
  using std::endl;

  Simulation::Simulation( SimulationParameters *par )
  {
    // Create the world object
    m_world = new World;

    // Create the grid object
    m_grid = new CubicGrid(par->gridWidth, par->gridHeight, par->gridDepth);

    // Let the world know about the grid
    m_world->setGrid(m_grid);

    // Create the agents for the simulation
    createAgents(m_grid->volume(), par->voltageSource, par->voltageDrain, par->defectPercentage);

    // Now initialise the potential from the external field
    updatePotentials( par->trapPercentage, par->deltaEpsilon );

    // precalculate potentials for interacting charges ( carriers, defects, traps )
    updateInteractionEnergies();

    // Set coulomb interactions
    m_coulombInteraction = par->coulomb;

    // Set charged defects
    m_chargedDefects = par->defectsCharged;

    // Set charged traps
    m_chargedTraps = par->trapsCharged;

    // Set charge on defects
    m_zDefect = par->zDefect;

    // Set charge on traps
    m_zTrap = par->zTrap;

    // Set simulation temperature
    m_temperatureKelvin = par->temperatureKelvin;

    // Calculate the number of charge carriers
    int nCharges = par->chargePercentage * double(m_grid->volume());
    setMaxCharges(nCharges);

    // Charge the grid up is specified
    if (par->gridCharge) seedCharges();

  }

  Simulation::~Simulation()
  {
    delete m_world;
    delete  m_grid;
    m_world    = 0;
    m_grid     = 0;
  }

  void Simulation::setMaxCharges(int n)
  {
   m_source->setMaxCharges(n);
   m_maxcharges = n;
  }

  bool Simulation::seedCharges()
  {
    // We randomly place all of the charges in the grid
    Grid *grid = m_world->grid();

    unsigned int nSites = grid->width() * grid->height() * grid->depth();
    int maxCharges = m_source->maxCharges();

    for (int i = 0; i < maxCharges; ) {
      // Randomly select a site, ensure it is suitable, if so add a charge agent
      unsigned int site = int(m_world->random() * (double(nSites) - 0.00001));
      if (grid->siteID(site) == 0 && grid->agent(site) == 0) {
        ChargeAgent *charge = new ChargeAgent(m_world, site, m_coulombInteraction, m_temperatureKelvin, m_zDefect, m_zTrap);
        m_world->charges()->push_back(charge);
        m_source->incrementCharge();
        ++i;
      }
    }
    return true;
  }

  void Simulation::setCoulombInteractions(bool enabled)
  {
   m_coulombInteraction = enabled;
  }
	
  void Simulation::setChargedDefects(bool on)
  {
   m_chargedDefects = on; 
  }
	
  void Simulation::setChargedTraps(bool ok)
  {
   m_chargedTraps = ok;
  }
	
  void Simulation::setZdefect(int zDefect)
  {
   m_zDefect = zDefect;
  }
	
  void Simulation::setZtrap(int zTrap)
  {
   m_zTrap = zTrap;
  }
	
  void Simulation::setTemperature(double temperatureKelvin)
  {
   m_temperatureKelvin = temperatureKelvin;
  }
  
  void Simulation::performIterations(int nIterations)
  {
    //	cout << "Entered performIterations function.\n";
    for (int i = 0; i <  nIterations; ++i) {
      // Attempt to transport the charges through the film
      QList<ChargeAgent *> &charges = *m_world->charges();

      // Use QtConcurrnet to parallelise the charge calculations
      QFuture<void> future = QtConcurrent::map(charges, Simulation::chargeAgentIterate);

      // We want to wait for it to finish before continuing on
      future.waitForFinished();

      // Now we are done with the charge movement, move them to the next tick!
      nextTick();

      // Begin by performing charge injection at the source
      unsigned int site = m_source->transport();
      //qDebug () << "Source transport returned site:" << site;

      if (site != errorValue) {
       ChargeAgent *charge = new ChargeAgent(m_world, site, m_coulombInteraction, m_temperatureKelvin, m_zDefect, m_zTrap);
       m_world->charges()->push_back(charge);
      }
    }
  }

  void Simulation::nextTick()
  {
    // Iterate over all sites to change their state
    QList<ChargeAgent *> &charges = *m_world->charges();
    for (int i = 0; i < charges.size(); ++i) {
      charges[i]->completeTick();
      // Check if the charge was removed - then we should delete it
      if (charges[i]->removed()) {
        delete charges[i];
        charges.removeAt(i);
        m_drain->acceptCharge(-1);
        m_source->decrementCharge();
        --i;
      }
    }
  }

  void Simulation::printGrid()
  {
    system("clear");
    unsigned int width  = m_world->grid()->width();
    unsigned int height = m_world->grid()->height();
    for (unsigned int j = 0; j < height; ++j) {
      cout << "||";
      for (unsigned int i = 0; i < width; ++i) {
        if (m_world->grid()->agent(i+j*width)) {// Charge is present
          if (m_world->grid()->siteID(i+j*width) == 0)
            cout << "*";
          else
            cout << "x";
        }
        else if (m_world->grid()->siteID(i+j*width) == 1)
          cout << "O";
        else
          cout << " ";
      }
      cout << "||\n";
    }
  }

  unsigned long Simulation::totalChargesAccepted()
  {
   return m_drain->acceptedCharges();
  }

  unsigned long Simulation::charges()
  {
   return m_world->charges()->size();
  }

  void Simulation::createAgents( unsigned int numAgents, double sourcePotential, double drainPotential, double defectPercent )
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
    for ( unsigned int i = 0; i < numAgents; ++i ) {
     if (m_world->random() < defectPercent)
     {
      m_world->grid()->setSiteID(i,1); //defect
      m_world->chargedDefects()->push_back(i); //Add defect to list - for charged defects
     }
     else 
     {
      m_world->grid()->setSiteID(i,0); //Charge carrier site
     }
    }
    // Add the source and the drain
    m_source = new SourceAgent(m_world, numAgents, sourcePotential);
    m_world->grid()->setAgent(numAgents, m_source);
    m_world->grid()->setSiteID(numAgents, 2);
    m_drain = new DrainAgent(m_world, numAgents+1, drainPotential);
    m_world->grid()->setAgent(numAgents+1, m_drain);
    m_world->grid()->setSiteID(numAgents+1, 3);

    // Now to assign nearest neighbours for the electrodes.
    vector<unsigned int> neighbors(0,0);

    // Loop over layers
    for ( unsigned int layer = 0; layer < m_grid->depth(); layer++ ) 
    {
     // Get column in this layer that is closest to the source
     vector<unsigned int> column_neighbors_in_layer = m_grid->col(0,layer);
     // Loop over this column
     for ( unsigned int column_site = 0; column_site < column_neighbors_in_layer.size(); column_site++ ) 
     {
      neighbors.push_back( column_neighbors_in_layer[column_site] );
     }
    }
    // Set the neighbors of the source
    m_source->setNeighbors(neighbors);
    neighbors.clear();

    // Loop over layers
    for ( unsigned int layer = 0; layer < m_grid->depth(); layer++ )
    {
     // Get column in this layer that is closest to the source
     vector<unsigned int> column_neighbors_in_layer = m_grid->col(m_grid->width()-1,layer);
     // Loop over this column
     for ( unsigned int column_site = 0; column_site < column_neighbors_in_layer.size(); column_site++ ) 
     {
      neighbors.push_back( column_neighbors_in_layer[column_site] );
     }
    }
    // Set the neighbors of the drain
    m_drain->setNeighbors(neighbors);
    neighbors.clear();
  }

  void Simulation::destroyAgents()
  {
    delete m_source;
    m_source = 0;
    delete m_drain;
    m_drain = 0;
  }

  void Simulation::updatePotentials(double trapPercent, double deltaEpsilon)
  {
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
    double tPotential = m * (double(width)+0.5) + c;	
    m_grid->setPotential(width*m_grid->height()*m_grid->depth()+1, tPotential);
  }

  void Simulation::updateInteractionEnergies()
  {
    //These values are used in Coulomb interaction calculations.
    TripleIndexArray& energies = m_world->interactionEnergies();
 
    // Currently hard coding a cut off of 50nm - no interactions beyond that added
    unsigned int cutoff = 50;

    energies.resize(cutoff,cutoff,cutoff);
    const double q = 1.60217646e-19; // Magnitude of charge on an electron

    // Now calculate the numbers we need
    for (unsigned int col = 0; col < cutoff; ++col){
     for (unsigned int row = 0; row < cutoff; ++row){
      for (unsigned int lay = 0; lay < cutoff; ++lay) {
       energies(col,row,lay)  = q / sqrt(col*col + row*row + lay*lay);
      }        
     }
    }
  }

  inline void Simulation::chargeAgentIterate(ChargeAgent *chargeAgent)
  {
    // This function performs a single iteration for a charge agent, only thread
    // safe calls can be made in this function. Other threads may access the
    // current state of the chargeAgent, but will not attempt to modify it.
    chargeAgent->transport();
  }

} // End namespace Langmuir
