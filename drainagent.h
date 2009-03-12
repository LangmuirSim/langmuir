/*
 *  DrainAgent - agent to act as a source of charges
 */

#ifndef DRAINAGENT_H
#define DRAINAGENT_H

#include "agent.h"

#include <vector>

namespace Langmuir
{

class DrainAgent : public Agent
{

public:
    DrainAgent(World *world, unsigned int site);
    DrainAgent(World *world, unsigned int site, double potential);
    ~DrainAgent();

    /**
     * Set the nearest neighbours of the agent.
     */
    virtual void setNeighbors(std::vector<Agent *> neighbors);

    /**
     * Set the potential of the source.
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

    /**
     * Perform a transport attempt
     */
    virtual Agent* transport();

    /**
     * Move on to the next time step.
     */
    virtual void completeTick()	{ }

private:
    std::vector<Agent *> m_neighbors;
    double m_potential;
};

} // End namespace Langmuir

#endif
