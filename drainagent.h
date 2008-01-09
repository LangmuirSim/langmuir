/*
 *  DrainAgent - agent to act as a source of charges
 */

#ifndef __DRAINAGENT_H
#define __DRAINAGENT_H

#include "agent.h"

#include <vector>

namespace Langmuir
{

class DrainAgent : public Agent
{

public:
	DrainAgent(unsigned int site);
	DrainAgent(unsigned int site, double potential);
	~DrainAgent();

	/**
	 * Set the nearest neighbours of the agent.
	 */
	virtual void setNeighbors(std::vector<Agent *> neighbors);

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

	/**
	 * Perform a transport attempt
	 */
	virtual unsigned int transport();
    
    /**
     * Move on to the next time step.
     */
    virtual void completeTick() { }

	/**
	 * Set the potential of the source.
	 */
	virtual void setPotential(double potential)
	{
		m_potential = potential;
	}

private:
	unsigned int m_site;
	std::vector<Agent *> m_neighbors;
	double m_potential;
};

} // End namespace Langmuir

#endif
