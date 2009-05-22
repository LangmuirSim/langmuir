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
                         double sourcePotential, double drainPotential)
  {
    m_world = new World;
    m_grid = new CubicGrid(width, height);
    m_world->setGrid(m_grid);

    // Create the agents for the simulation
    createAgents(width * height, sourcePotential, drainPotential);
    // The e-field is constant between the electrodes - V / d
    m_world->setEField((drainPotential - sourcePotential) / (width*1.0e-9));

    // Now inialise the potentials
    updatePotentials();
  }

  Simulation::~Simulation()
  {
//    destroyAgents();
    delete m_world;
    m_world = 0;
    delete m_grid;
    m_grid = 0;
  }

  void Simulation::performIterations(int nIterations)
  {
    //	cout << "Entered performIterations function.\n";
    for (int i = 0; i <  nIterations; ++i) {
      // Attempt to transport the charges through the film
      vector<ChargeAgent *> &charges = *m_world->charges();
      cout << "Charges in system: " << charges.size() << endl;

      // Use QtConcurrnet to parallelise the charge calculations
      QFuture<void> future = QtConcurrent::map(charges,
                                               Simulation::chargeAgentIterate);
      // We want to wait for it to finish before continuing on
      future.waitForFinished();

      // Now we are done with the charge movement, move them to the next tick!
      nextTick();

      // Begin by performing charge injection at the source
      unsigned int site = m_source->transport();
      qDebug () << "Source transport returned site:" << site;
      if (site != errorValue) {
        cout << "New charge injected! " << site << endl;
        m_world->charges()->push_back(new ChargeAgent(m_world, site));
      }

    }
  }

  void Simulation::nextTick()
  {
    // Iterate over all sites to change their state
    vector<ChargeAgent *> &charges = *m_world->charges();
    for (vector<ChargeAgent *>::const_iterator it = charges.begin();
         it != charges.end(); ++it) {
      (*it)->completeTick();
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
        if (m_world->grid()->agent(i+j*width)) // Charge is present
          cout << "*";
        else if (m_world->grid()->siteID(i+j*width) == 1)
          cout << "O";
//      else if (charges[i]->charge() == 0 && charges[i]->pBarrier() > 0.599)
//        cout << "X";
        else
          cout << " ";
      }
      cout << "||\n";
    }
  }

  void Simulation::createAgents(unsigned int numAgents, double sourcePotential,
                                double drainPotential)
  {
    // Add the normal agents
    for (unsigned int i = 0; i < numAgents; ++i) {
      // Mix some trap sites in
      if (m_world->random() > 0.95)
        m_world->grid()->setSiteID(i, 1);
    }
    // Add the source and the drain
    m_source = new SourceAgent(m_world, numAgents, sourcePotential);
    m_world->grid()->setAgent(numAgents, m_source);
    m_drain = new DrainAgent(m_world, numAgents+1, drainPotential);
    m_world->grid()->setAgent(numAgents+1, m_drain);
    // Now to assign nearest neighbours for the electrodes.
    vector<unsigned int> neighbors = m_grid->col(0);
    cout << "Source neighbors: ";
    for (unsigned int i = 0; i < neighbors.size(); ++i)
      cout << neighbors[i] << " ";
    cout << endl;
    m_source->setNeighbors(neighbors);
    neighbors.clear();

    neighbors = m_grid->col(m_grid->width()-1);
    m_drain->setNeighbors(neighbors);
    neighbors.clear();
  }

  void Simulation::destroyAgents()
  {
    for (int i = 0; i < m_charges.size(); ++i) {
      delete m_charges[i];
      m_charges[i] = 0;
    }
    m_charges.clear();
    delete m_source;
    m_source = 0;
    delete m_drain;
    m_drain = 0;
  }

  void Simulation::updatePotentials()
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
        m_grid->setPotential(*j, tPotential);
      }
      cout << "Row " << i << ", potential = " << tPotential << endl;
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
