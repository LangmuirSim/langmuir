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
	std::vector<int> m_charges;

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

};

} // End namespace Langmuir

#endif
