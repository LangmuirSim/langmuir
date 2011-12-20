#ifndef SIMULATION_H
#define SIMULATION_H

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
     * Releases the world object.
     */
    virtual ~Simulation();

    /**
     * @brief Perform Iterations. 
     *
     * Perform so many iterations of the simulation.
     * @param nIterations number of iterations to perform.
     */
    virtual void performIterations(int nIterations) = 0;

    /**
     * @brief World access.
     *
     * Retrieve the world object - can be used alter world properties.
     */
    World *world();

  protected:

    /**
     * @brief Add Defects
     *
     * Defects are sites that carriers can not transport to
     */
    virtual void createDefects();

    /**
     * @brief Place charges on grid.
     *
     * Seed the grid with the number of charges specified by setMaxCharges.
     */
    virtual void seedCharges();

    /**
     * @brief Place drains (left and right) on grid.
     *
     * Drains accept charges based on probability parameters
     */
    virtual void setUpDrains();

    /**
     * @brief Place sources (left and right) on grid.
     *
     * Sources inject charges based on probability parameters
     */
    virtual void setUpSources();

    /**
     * @brief World pointer.
     */
    World *m_world;

    /**
     * @brief Number of iterations performed
     */
    int m_tick;

    /**
      * @brief a unique id for this simulation
      */
    int m_id;
  };

  inline World* Simulation::world ()
  {
      return m_world;
  }
} // End namespace Langmuir

#endif
