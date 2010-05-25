#include "simulation.h"

#include "world.h"
#include "cubicgrid.h"
#include "chargeagent.h"
#include "sourceagent.h"
#include "drainagent.h"

#include <iostream>
#include <cstdlib>

#include <QtCore/QFuture>
#include <QtCore/QtConcurrentMap>

namespace Langmuir
{
  using std::vector;
  using std::cout;
  using std::endl;

  Simulation::Simulation(unsigned int width, unsigned int height,
                         double sourcePotential, double drainPotential,
                         double defectPercent, double trapPercent, double deltaEpsilon)
      : m_coulombInteraction(true), m_chargedDefects(true)
  {
    m_world = new World;
    m_grid = new CubicGrid(width, height);
    m_world->setGrid(m_grid);

    // Create the agents for the simulation
    createAgents(width * height, sourcePotential, drainPotential, defectPercent);
    // The e-field is constant between the electrodes - V / d
    m_world->setEField((drainPotential - sourcePotential) / (width*1.0e-9));

    // Now initialise the potentials
    updatePotentials(trapPercent, deltaEpsilon);
    updateInteractionEnergies();
	  
	// Reset potentials if the trap percentage is varied
	/*if (trapPercent > 0)
	{
		updatePotentials(trapPercent, m_deltaEpsilon);
	}*/
  }

  Simulation::~Simulation()
  {
//    destroyAgents();
    delete m_world;
    m_world = 0;
    delete m_grid;
    m_grid = 0;
  }

  void Simulation::setMaxCharges(int n)
  {
    m_source->setMaxCharges(n);
  }

  bool Simulation::seedCharges()
  {
    // We randomly place all of the charges in the grid
    Grid *grid = m_world->grid();

    unsigned int nSites = grid->width() * grid->height();
    int maxCharges = m_source->maxCharges();

    for (int i = 0; i < maxCharges; ) {
      // Randomly select a site, ensure it is suitable, if so add a charge agent
      unsigned int site = int(m_world->random() * (double(nSites) - 0.00001));
      if (grid->siteID(site) == 0 && grid->agent(site) == 0) {
        ChargeAgent *charge = new ChargeAgent(m_world, site, m_coulombInteraction, m_temperatureKelvin, m_zDefect);
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
	
  void Simulation::setZdefect(int zDefect)
	{
	   m_zDefect = zDefect;
	}
	
  void Simulation::setTemperature(double temperatureKelvin)
	{
		m_temperatureKelvin = temperatureKelvin;
	}
	
  /*void Simulation::setDeltaEpsilon(double deltaEpsilon)
	{
		m_deltaEpsilon = deltaEpsilon;
	}*/
  
  void Simulation::performIterations(int nIterations)
  {
    //	cout << "Entered performIterations function.\n";
    for (int i = 0; i <  nIterations; ++i) {
      // Attempt to transport the charges through the film
      QList<ChargeAgent *> &charges = *m_world->charges();

      // Use QtConcurrnet to parallelise the charge calculations
      QFuture<void> future = QtConcurrent::map(charges,
                                               Simulation::chargeAgentIterate);
      // We want to wait for it to finish before continuing on
      future.waitForFinished();

      // Now we are done with the charge movement, move them to the next tick!
      nextTick();

      // Begin by performing charge injection at the source
      unsigned int site = m_source->transport();
      //qDebug () << "Source transport returned site:" << site;
      if (site != errorValue) {
//        cout << "New charge injected! " << site << endl;
        ChargeAgent *charge = new ChargeAgent(m_world, site, m_coulombInteraction, m_temperatureKelvin, m_zDefect);
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
    unsigned int width = m_world->grid()->width();
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

  void Simulation::createAgents(unsigned int numAgents, double sourcePotential,
                                double drainPotential, double defectPercent)
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

    // Add the normal agents
    for (unsigned int i = 0; i < numAgents; ++i) {
      // Mix some defects in
      if (m_world->random() < defectPercent) // Defect 
	  {
        m_world->grid()->setSiteID(i, 1);
		m_world->chargedDefects()->push_back(i);
	  }
	else // Charge carrier site
        m_world->grid()->setSiteID(i, 0);
    }
    // Add the source and the drain
    m_source = new SourceAgent(m_world, numAgents, sourcePotential);
    m_world->grid()->setAgent(numAgents, m_source);
    m_world->grid()->setSiteID(numAgents, 2);
    m_drain = new DrainAgent(m_world, numAgents+1, drainPotential);
    m_world->grid()->setAgent(numAgents+1, m_drain);
    m_world->grid()->setSiteID(numAgents+1, 3);
    // Now to assign nearest neighbours for the electrodes.
    vector<unsigned int> neighbors = m_grid->col(0);
//    cout << "Source neighbors: " << neighbors.size() << endl;
//    for (unsigned int i = 0; i < neighbors.size(); ++i)
//      cout << neighbors[i] << " ";
//    cout << endl;
    m_source->setNeighbors(neighbors);
    neighbors.clear();

    neighbors = m_grid->col(m_grid->width()-1);
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
      double tPotential = m * (double(i)+0.5) + c;
	  vector<unsigned int> neighbors = m_grid->col(i);
      for (vector<unsigned int>::iterator j = neighbors.begin();
           j != neighbors.end(); j++) {
		  // We can randomly tweak a site energy
		  if (m_world->random() < trapPercent)
		  {
			  m_grid->setPotential(*j, (tPotential + deltaEpsilon));
//			  cout << " Site " a<< *j << " deltaE " << deltaEpsilon << " Potential " << tPotential << endl;
		  }
		  else
		  {
			  m_grid->setPotential(*j, tPotential);
		  }
      }
//      cout << "Row " << i << ", potential = " << tPotential << endl;
    }
	
	// Set the potential of the drain site
    double tPotential = m * (double(width)+0.5) + c;	
	m_grid->setPotential(width*m_grid->height()+1, tPotential);

  }

  void Simulation::updateInteractionEnergies()
  {
    // These values are used in Coulomb interaction calculations.
    Eigen::MatrixXd *energies = m_world->interactionEnergies();

    // Currently hard coding a cut off of 50nm - no interactions beyond that added
    unsigned int cutoff = 50;
    energies->resize(cutoff, cutoff);
    const double q = 1.60217646e-19; // Magnitude of charge on an electron
    // Now calculate the numbers we need
    for (unsigned int i = 0; i < cutoff; ++i)
      for (unsigned int j = 0; j < cutoff; ++j)
        (*energies)(i, j) = q / sqrt(i*i + j*j);
  }

  inline void Simulation::chargeAgentIterate(ChargeAgent *chargeAgent)
  {
    // This function performs a single iteration for a charge agent, only thread
    // safe calls can be made in this function. Other threads may access the
    // current state of the chargeAgent, but will not attempt to modify it.
    chargeAgent->transport();
  }

} // End namespace Langmuir
