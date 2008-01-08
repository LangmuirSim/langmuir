/*
 *  Simulation - simulation management class
 *
 */

#ifndef __SIMULATION_H
#define __SIMULATION_H

#include "grid.h"
#include "agent.h"

#include <vector>

namespace Langmuir {

  class Simulation {

  public:
    Simulation(unsigned int width, unsigned int height);
    ~Simulation();    

  private:
    /**
     * Grid instance - used for nearest neighbours, distances etc
     */
    Grid *m_grid;

    /**
     * Vector containing a list of all agents. The source is num_agents and the
     * drain is num_agents + 1.
     */
    std::vector<Agent *> m_agents;
    std::vector<int> m_charges;

    void createAgents(unsigned int num_agents);
    void destroyAgents();

  };

} // End namespace Langmuir

#endif
