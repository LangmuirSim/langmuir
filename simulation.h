/*
 *  Simulation - simulation management class
 *
 */

#ifndef SIMULATION_H
#define SIMULATION_H

#include <QList>

namespace Langmuir
{

  class World;
  class Grid;
  class Agent;
  class SourceAgent;
  class DrainAgent;
  class ChargeAgent;
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
     * Vector containing a list of all charge agents.
     */
    QList<Agent *> m_charges;

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

    /**
     * Private static member function - this function performs the majority of
     * calculations for each iteration. These calculations are performed in
     * threads and then any joining necessary is done at the end.
     */
    static void chargeAgentIterate(ChargeAgent *chargeAgent);

  };

} // End namespace Langmuir

#endif
