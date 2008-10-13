/*
 *  HoppingAgent - agent performing hopping...
 *
 */

#ifndef __HOPPINGAGENT_H
#define __HOPPINGAGENT_H

#include "agent.h"

#include <vector>

namespace Langmuir
{

class Grid;
class Rand;

class HoppingAgent : public Agent
{

public:
	HoppingAgent(unsigned int site, const Grid* grid);
	~HoppingAgent();

	/**
	 * Set the nearest neighbours of the agent.
	 */
	virtual void setNeighbors(std::vector<Agent *> neighbors);

    /**
     * Set the potential of this agent.
     */
  virtual void setPotential(double potential) { m_potential = potential; }

	/**
	 * Get the potential of this agent...
	 */
	virtual double potential();

	/**
	 * Attempt to move a charge to this agent. If the charge is accepted then
	 * this agent will store that charge in its future state, otherwise the
	 * attempted transfer failed and the charge will not be stored.
	 * @return true if accepted, false if not.
	 */
	virtual bool acceptCharge(int charge);

	/**
	 * Returns the charge of this node.
	 */
	virtual int charge();

  virtual int fCharge();

	/**
	 * Perform a transport attempt
	 */
	virtual Agent* transport();

	/**
	 * Move on to the next time step.
	 */
	virtual void completeTick();

	/**
	 * Return the site number - mainly for debugging purposes...
	 */
	virtual unsigned int site() { return m_site; }

private:
	unsigned int m_site;
	const Grid* m_grid;
	std::vector<Agent *> m_neighbors;
	double m_potential;
	int m_charge, m_fCharge;
	Rand *m_rand;
	double m_pBarrier;
};

} // End namespace Langmuir

#endif
