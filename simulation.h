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
    /**
     * Set up the basic parameters of the simulation.
     * @param width The width of the grid.
     * @param height The height of the grid.
     * @param sourcePotential The potential at the source electrode.
     * @param drainPotential The potential at the drain electrode.
     */
    Simulation(unsigned int width, unsigned int height,
               double sourcePotential, double drainPotential,
               double defectPercent = 0.0, double trapPercent = 0.0, double deltaEpsilon = 0.0);

    /**
     * Destructor.
     */
    ~Simulation();

    /**
     * Set the number of charges to maintain in the system.
     */
    void setMaxCharges(int n);

    /**
     * Seed the grid with the number of charges specified by setMaxCharges.
     */
    bool seedCharges();

    /**
     * Account for Coulomb interaction between charges.
     */
    void setCoulombInteractions(bool enabled);
	  
	// Account for charged defects
	
	void setChargedDefects(bool on);
	  
	// Set the charge of the defect
	  
	void setZdefect(int zDefect);
	  
    // Set the simulation temperature
	  
	void setTemperature(double temperatureKelvin);
	  
    /**
     * Call this function to perform n iterations.
     */
	  
    void performIterations(int nIterations);

    /**
     * Used to print an ASCII grid to the terminal - low tech visualization.
     */
    void printGrid();

    /**
     * Retrieve a charge drain count - total number of charges the drain has
     * received since the simulation started.
     */
    unsigned long totalChargesAccepted();

    /**
     * Retrieve the current number of charges in the system.
     */
    unsigned long charges();

    /**
     * Retrieve the world object - can be used alter world properties.
     */
    World * world();

  private:
    bool m_coulombInteraction; // Should Coulomb interactions be taken into account?
	bool m_chargedDefects;  // Enable charged defects
	int m_zDefect; // Set the charge on the defect
	double m_temperatureKelvin; // Set the simulation temperature

    World *m_world;
    /**
     * Grid instance - used for nearest neighbours, distances etc
     */
    Grid *m_grid;

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
                      double drainPotential, double defectPercent);

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
    void updatePotentials(double trapPercent, double deltaEpsilon);

    /**
     * Helper function - precalculate the interaction energies of charges at
     * different sites. This is a vector of size width + length denoting the x,
     * y displacement of the two sites under consideration. These values
     * reduce necessary calculations in tight loops when considering Coulomb
     * interactions.
     */
    void updateInteractionEnergies();

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

  inline World * Simulation::world()
  {
    return m_world;
  }

} // End namespace Langmuir

#endif
