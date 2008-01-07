#include "simulation.h"

using namespace std;

namespace Langmuir {

  Simulation::Simulation(unsigned int width, unsigned int height)
  {
    m_grid = new CubicGrid(width, height);
    
    // Create the agents for the simulation
    createAgents(width * height);
  }
  
  Simulation::~Simulation()
  {
    delete m_grid;
    m_grid = 0;
  }
  
  void Simulation::createAgents(unsigned int num_agents)
  {
    m_agents.resize(num_agents+2);
    // Add the normal agents
    for (unsigned int i = 0; i < num_agents; i++)
    {
      m_agents[i] = new HoppingAgent;
    }
    // Add the source and the drain
    m_agents[i+1] = new SourceAgent;
    m_agents[i+2] = new DrainAgent;
    
    for (unsigned int i = 0; i < num_agents; i++)
    {
      vector<unsigned int> neighbors = m_grid->getNeighbors(i);
      vector<Agent *> pNeighbors;
      for (unsigned int j = 0; j < neighbors.size(); j++)
      {
        pNeighbors.push_back(m_agents[neighbors[j]]);
      }
      m_agents[i]->setNeighbors(pNeighbors);
    }
  }
  
  void Simulation::destroyAgents()
  {
    
  }
  
} // End namespace Langmuir
