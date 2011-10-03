#ifndef SIMULATION_H
#define SIMULATION_H

extern int globalstep;

#include <QList>

namespace Langmuir
{

  class World;
  class Grid;
  class Agent;
  class SourceAgent;
  class DrainAgent;
  class ChargeAgent;
  class Potential;
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
    Simulation (SimulationParameters * par, int id = 0);

    /**
     * @brief Destructor.
     *
     * Releases the random number generator.
     */
    ~Simulation ();

    /**
     * @brief Place charges on grid.
     *
     * Seed the grid with the number of charges specified by setMaxCharges.
     */
    bool seedCharges ();

    /**
     * @brief Perform Iterations. 
     *
     * Perform so many iterations of the simulation.
     * @param nIterations number of iterations to perform.
     */
    void performIterations (int nIterations);

    /**
     * @brief Perform injection.
     *
     * Allow source to perform this many injections.
     * @param nInjections number of injections to perform.
     */
    void performInjections (int nInjections=1);

    /**
     * @brief 2D visualize
     *
     * Used to print an ASCII grid to the terminal - low tech visualization.
     */
    void printGrid ();

    /**
     * @brief Drain Count.
     *
     * Retrieve a charge drain count - total number of charges the drain has
     * received since the simulation started.
     */
    unsigned long totalChargesAccepted ();

    /**
     * @brief ncharges
     *
     * Retrieve the current number of charges in the system.
     */
    unsigned long charges ();

    /**
     * @brief ncharges
     *
     * Retrieve the current number of charges in the system.
     */
    int getMaxCharges ()
    {
      return m_maxcharges;
    }

    /**
     * @brief World access.
     *
     * Retrieve the world object - can be used alter world properties.
     */
    World *world ();

    /**
     * @brief Access the Grid
     *
     * Get the address of the Simulation Grid.
     */
    Grid *getGrid ()
    {
      return m_grid;
    }

    /**
     * @brief the total steps performed
     *
     */
    int getTick();

  private:

    /**
      * @brief a unique id for this simulation
      */
    int m_id;

    /**
     * @brief Charge Defect charge.
     *
     * The charge on traps in the simulation.
     */
    int m_maxcharges;

    /**
     * @brief World pointer.
     *
     * Address of the world object used for the simulation.
     */
    World *m_world;

    /**
     * @brief Grid pointer.
     *
     * Address of the grid object used for the simulation.
     */
    Grid *m_grid;

    /**
     * @brief Parameter pointer.
     *
     * Address of the parameter object used for the simulation.
     */
    SimulationParameters *m_parameters;

    /**
     * @brief Potential calculator
     *
     * Address of the potential object for the simulation.
     */
    Potential *m_potential;

    /**
     * @brief Source pointer.
     *
     * Address of the source object used for the simulation.
     */
    SourceAgent *m_source;

    /**
     * @brief Drain pointer.
     *
     * Address of the drain object used for the simulation.
     */
    DrainAgent *m_drain;

    int m_tick;

    /**
     * @brief Generate Agents.
     *
     * Create all the agents in the simulation and set up their initial state.
     */
    void createAgents ();

    /**
     * @brief Release agent memory.
     *
     * Destroy the agents once we are done with them.
     */
    void destroyAgents ();

    /**
     * @brief update the potential at each site.
     *
     * This only needs to be done when initializing the system or when the source and/or drain potential is modified.
     * Right now we are assuming that there is one source, one drain and that they are parallel.
     * This is a good assumption as it simplifies many things in the simulation and also applies to many different real world devices.
     * Making a more generic implementation would be more difficult and slower.
     */
    void updatePotentials ();
    void heteroTraps ();

    // Prints grid sites
    void gridImage();

    /**
     * @brief Precalculate interaction energies.
     * 
     * precalculate the interaction energies of charges at different sites. 
     * This is a vector of size width + length + depth denoting the x, y displacement of the two sites under consideration. 
     * These values reduce necessary calculations in tight loops when considering Coulomb interactions.
     */
    void updateInteractionEnergies ();

    /**
     * @brief Next simulation Tick.
     *
     * Move on to the next time tick - change to the future states, clean up.
     */
    void nextTick ( );

    /**
     * @brief charge agent iterations.
     *
     * Private static member function - this function performs the majority of
     * calculations for each iteration. These calculations are performed in
     * threads and then any joining necessary is done at the end.
     * @param chargeAgent pointer to charge agent.
     */
    static void chargeAgentIterate (ChargeAgent * chargeAgent);

    static void chargeAgentChooseFuture( ChargeAgent * chargeAgent );

    /**
      * @brief finishes what chargeAgentIterate fails to do when OpenCL is used
      *
      * In between generating future sites, and deciding if those future sites
      * can be moved to, we calculate coulomb interactions.  We need to gather
      * ALL the future sites before sending them to a compute device before
      * before deciding the outcome.
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
