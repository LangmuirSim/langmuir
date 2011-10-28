#ifndef SIMULATION_H
#define SIMULATION_H

extern int globalstep;

#include <QtCore>

namespace Langmuir
{
  class World;
  class Grid;
  class Agent;
  class Potential;
  class DrainAgent;
  class SourceAgent;
  class ChargeAgent;
  struct SimulationParameters;
  /**
    *  @class Simulation
    *  @brief Simulation management.
    *
    *  Organizes simulation and performs relevent tasks.
    *  @date 06/07/2010
    */
  class Simulation
  {

  public:
    /**
     * @brief Constructor.
     *
     * Set up the basic parameters of the simulation.
     * @param par parameter struct
     */
    Simulation(SimulationParameters * par, int id = 0);

    /**
     * @brief Destructor.
     *
     * Releases the random number generator.
     */
    ~Simulation();

    /**
     * @brief Add Defects
     *
     * Defects are sites that carriers can not transport to
     */
    void createDefects();

    /**
     * @brief Add Source
     *
     * The source injects carriers
     */
    void createSource();

    /**
     * @brief Add Drain
     *
     * The drain acccepts carriers
     */
    void createDrain();

    /**
     * @brief Place charges on grid.
     *
     * Seed the grid with the number of charges specified by setMaxCharges.
     */
    void seedCharges();

    /**
     * @brief Perform Iterations. 
     *
     * Perform so many iterations of the simulation.
     * @param nIterations number of iterations to perform.
     */
    void performIterations (int nIterations);

    /**
     * @brief World access.
     *
     * Retrieve the world object - can be used alter world properties.
     */
    World *world();

    /**
     * @brief the total steps performed
     *
     */
    int getTick();

  private:

    /**
     * @brief World pointer.
     */
    World *m_world;

    /**
      * @brief a unique id for this simulation
      */
    int m_id;

    /**
     * @brief Number of iterations performed
     */
    int m_tick;

    /**
     * @brief inject charges into the system
     * @param nInjections number of injections to perform.
     */
    void performInjections (int nInjections=1);

    /**
     * @brief next simulation Tick.
     */
    void nextTick ( );

    /**
     * @brief charge agent transport for QtConcurrent.
     */
    static void chargeAgentIterate( ChargeAgent * chargeAgent );

    /**
     * @brief charge agent transport (choose future site) for QtConcurrent.
     */
    static void chargeAgentChooseFuture( ChargeAgent * chargeAgent );

    /**
     * @brief charge agent transport (apply metropolis) for QtConcurrent.
     */
    static void chargeAgentDecideFuture( ChargeAgent * chargeAgent );
  };

  inline World *Simulation::world ()
  {
    return m_world;
  }

  inline int Simulation::getTick()
  {
      return m_tick;
  }
}                                // End namespace Langmuir

#endif
