/*
 *  Simulation - simulation management class
 *
 */

#ifndef __SIMULATION_H
#define __SIMULATION_H

#include <vector>

namespace Langmuir
{

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

	void createAgents(unsigned int num_agents, double sourcePotential, double drainPotential);
	void destroyAgents();
	
	void nextTick(); // Move to the next time tick - change to fStates...

};

} // End namespace Langmuir

#endif
