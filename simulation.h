/*
 *  Simulation - simulation management class
 *
 */

#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>

namespace Langmuir
{

  class World;
  class Grid;
  class Agent;
  class SourceAgent;
  class DrainAgent;
  class Simulation
  {

  public:
    Simulation(unsigned int width, unsigned int height, double sourcePotential,
               double drainPotential);
    ~Simulation();

    void performIterations(int nIterations);

    void printGrid();

  private:
    World *m_world;
    /**
     * Grid instance - used for nearest neighbours, distances etc
     */
    Grid *m_grid;

    /**
     * Vector containing a list of all agents. The source is num_agents and the
     * drain is num_agents + 1.
     */
    std::vector<Agent *> m_agents;

    /**
     * Vector containing a list of the index of each site with a charge on it.
     */
    std::vector<Agent *> m_charges;
    std::vector<Agent *> m_fCharges;

    /**
     * Pointer to the source.
     */
    SourceAgent *m_source;

    /**
     * Pointer to the drain.
     */
    DrainAgent *m_drain;

    /**
     * Create all the agents in the simulation and set up their initial state.
     */
    void createAgents(unsigned int num_agents, double sourcePotential,
                      double drainPotential);

    /**
     * Destroy the agents once we are done with them.
     */
    void destroyAgents();

    /**
     * Helper function - update the potential at each site. This only needs to
     * be done when initialising the system or when the source and/or drain
     * potential is modified.
     *
     * NOTE: Right now we are assuming that there is one source, one drain and
     * that they are parallel. This is a good assumption as it simplifies many
     * things in the simulation and also applies to many different real world
     * devices. Making a more generic implementation would be more difficult and
     * slower.
     */
    void updatePotentials();

    /**
     * Move on to the next time tick - change to the future states, clean up.
     */
    void nextTick();

  };

} // End namespace Langmuir

#endif
