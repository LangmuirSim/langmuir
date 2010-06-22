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
    Simulation( SimulationParameters *par );

    /**
     * @brief Destructor.
     *
     * Releases the random number generator.
     */
    ~Simulation();

    /**
     * @brief Set number of charges.
     *
     * Set the number of charges to maintain in the system.
     * @param N the number of charges to use.
     */
    void setMaxCharges(int n);

    /**
     * @brief Place charges on grid.
     *
     * Seed the grid with the number of charges specified by setMaxCharges.
     */
    bool seedCharges();

    /**
     * @brief Coulomb interactions
     *
     * Toggle Coulomb interactions for charge carriers.
     * @param enabled boolean value for option.
     */
    void setCoulombInteractions(bool enabled);

    /**
     * @brief Defects
     *
     * Toggle interaction for charge carriers with charged defects.
     * @param enabled boolean value for option.
     */  
    void setChargedDefects(bool on);
	
    /**
     * @brief Charged Traps
     *
     * Enables interaction for charge carriers with charged traps
     * @param enabled boolean value for option.
     */
    void setChargedTraps(bool ok);
	  
    /**
     * @brief Defect charge
     *
     * Sets the magnitude of the charge on defects.
     * @param Zdefect charge of defects.
     */	  
    void setZdefect(int zDefect);

    /**
     * @brief Trap charge
     *
     * Sets the magnitude of the charge on traps.
     * @param Ztrap charge of traps.
     */	  
    void setZtrap(int zTrap);

    /**
     * @brief Simulation temperature.
     *
     * Set the simulation temperature to be used in the metropolis criterion.
     * @param temperatureKelvin temperature to use in Kelvin units.
     */
    void setTemperature(double temperatureKelvin);
	  
    /**
     * @brief Perform Iterations. 
     *
     * Perform so many iterations of the simulation.
     * @param nIterations number of iterations to perform.
     */
    void performIterations(int nIterations);

    /**
     * @brief 2D visualize
     *
     * Used to print an ASCII grid to the terminal - low tech visualization.
     */
    void printGrid();

    /**
     * @brief Access the Grid
     *
     * Get the address of the Simulation Grid.
     */
    Grid * getGrid() { return m_grid; }

    /**
     * @brief Drain Count.
     *
     * Retrieve a charge drain count - total number of charges the drain has
     * received since the simulation started.
     */
    unsigned long totalChargesAccepted();

    /**
     * @brief ncharges
     *
     * Retrieve the current number of charges in the system.
     */
    unsigned long charges();

    /**
     * @brief ncharges
     *
     * Retrieve the current number of charges in the system.
     */
    int getMaxCharges()
    {
     return m_maxcharges; 
    }

    /**
     * @brief World access.
     *
     * Retrieve the world object - can be used alter world properties.
     */
    World * world();

  private:

    /**
     * @brief Coulomb Interactions.
     *
     * Boolean allowing Coulomb interactions to be calculated.
     */
    bool m_coulombInteraction;

    /**
     * @brief Defect presence.
     *
     * Boolean allowing charged defects to be present in the simulation.
     */
    bool m_chargedDefects;
	  
    /**
     * @brief Charge trap presence.
     *
     * Boolean allowing trap sites to have a charge
     */
    bool m_chargedTraps; 

    /**
     * @brief Charge Defect charge.
     *
     * The charge on defects in the simulation.
     */
    int m_zDefect;
	  
    /**
     * @brief Charge Defect charge.
     *
     * The charge on traps in the simulation.
     */
    int m_zTrap;

    /**
     * @brief Simulation temperature.
     *
     * The temperature to perform the simulation at.
     */
    double m_temperatureKelvin;

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

    /**
     * /brief Generate Agents.
     *
     * Create all the agents in the simulation and set up their initial state.
     * @param agents the number of agents.
     * @param sourcePotential the potential of the source agent.
     * @param drainPotential the potential of the drain agent.
     * @param defectPercent the percent of defects present.
     */
    void createAgents(unsigned int num_agents, double sourcePotential, double drainPotential, double defectPercent);

    /**
     * @brief Release agent memory.
     *
     * Destroy the agents once we are done with them.
     */
    void destroyAgents();

    /**
     * @brief update the potential at each site.
     *
     * This only needs to be done when initialising the system or when the source and/or drain potential is modified.
     * Right now we are assuming that there is one source, one drain and that they are parallel. 
     * This is a good assumption as it simplifies many things in the simulation and also applies to many different real world devices.
     * Making a more generic implementation would be more difficult and slower.
     * @param trapPercent the percent of traps present.
     * @param deltaEpsilon the energy of a trap.
     */
    void updatePotentials( SimulationParameters *par );
    //void updatePotentials(double trapPercent, double deltaEpsilon);

    /**
     * @brief Precalculate interaction energies.
     * 
     * precalculate the interaction energies of charges at different sites. 
     * This is a vector of size width + length + depth denoting the x, y displacement of the two sites under consideration. 
     * These values reduce necessary calculations in tight loops when considering Coulomb interactions.
     */
    void updateInteractionEnergies();

    /**
     * @brief Next simulation Tick.
     *
     * Move on to the next time tick - change to the future states, clean up.
     */
    void nextTick();

    /**
     * @brief charge agent iterations.
     *
     * Private static member function - this function performs the majority of
     * calculations for each iteration. These calculations are performed in
     * threads and then any joining necessary is done at the end.
     * @param chargeAgent pointer to charge agent.
     */
    static void chargeAgentIterate(ChargeAgent *chargeAgent);

  };

  inline World * Simulation::world()
  {
    return m_world;
  }

} // End namespace Langmuir

#endif
