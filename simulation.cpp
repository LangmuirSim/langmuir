#include "simulation.h"

#include "world.h"
#include "cubicgrid.h"
#include "chargeagent.h"
#include "sourceagent.h"
#include "drainagent.h"

#include <iostream>
#include <cstdlib>

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
  }

  Simulation::~Simulation()
  {
    delete m_world;
    m_world = 0;
    delete m_grid;
    m_grid = 0;
    destroyAgents();
  }

  void Simulation::performIterations(int nIterations)
  {
    //	cout << "Entered performIterations function.\n";
    for (int i = 0; i <  nIterations; ++i) {
      // Attempt to transport the charges through the film
      vector<ChargeAgent *> &charges = *m_world->charges();
      cout << "Charges in system: " << charges.size() << endl;
      for (vector<ChargeAgent *>::const_iterator it = charges.begin();
           it != charges.end(); ++it) {
        (*it)->transport();
      }

      // Begin by performing charge injection at the source
      unsigned int site = m_source->transport();
      if (site != errorValue) {
        cout << "New charge injected! " << site << endl;
        m_world->charges()->push_back(new ChargeAgent(m_world, site));
      }
      else
        cout << "Simulate loop returned " << site << endl;

      if (m_world->grid()->agent(15))
      cout << m_world->grid()->agent(15) << "->"
           << m_world->grid()->agent(15)->site();

      // Now we are done with this iteration - move on to the next tick!
      nextTick();
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
//    m_agents.resize(numAgents+2);
    // Add the normal agents
    for (unsigned int i = 0; i < numAgents; ++i) {
      // Mix some trap sites in
      if (m_world->random() > 0.95)
        m_world->grid()->setSiteID(i, 1);
    }
    // Add the source and the drain
    m_source = new SourceAgent(m_world, numAgents, sourcePotential);
//    m_agents[num_agents] = m_source;
    m_world->grid()->setAgent(numAgents, m_source);
    m_drain = new DrainAgent(m_world, numAgents+1, drainPotential);
//    m_agents[num_agents+1] = m_drain;
    m_world->grid()->setAgent(numAgents+1, m_drain);
    // Now to assign nearest neighbours for the electrodes.
    vector<unsigned int> neighbors = m_grid->col(0);
    cout << "Source neighbors: ";
    for (int i = 0; i < neighbors.size(); ++i)
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
    for (unsigned int i = 0; i < m_agents.size(); i++)
    {
      delete m_agents[i];
      m_agents[i] = 0;
    }
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
    for (int i = 0; i < width; i++)
    {
      double tPotential = m * (double(i)+0.5) + c;
      vector<unsigned int> neighbors = m_grid->col(i);
      for (vector<unsigned int>::iterator j = neighbors.begin();
      j != neighbors.end(); j++)
      {
        //            m_agents[*j]->setPotential(tPotential);
      }
      cout << "Row " << i << ", potential = " << tPotential << endl;
    }
  }

} // End namespace Langmuir
