/*
 *  Agent - pure virtual base class to inherited by all agent classes
 *
 */

#ifndef __AGENT_H
#define __AGENT_H

#include <vector>

namespace Langmuir{
  
  class Agent {

  public:
    virtual ~Agent() { }
    
    /**
     * Set the nearest neighbours of the agent.
     */
    virtual void setNeighbors(std::vector<Agent *> neighbors) = 0;

    /**
     * Set the potential of this agent.
     */
    virtual void setPotential(double potential) = 0;
    
    /**
     * Get the potential of this agent.
     */
    virtual double potential() = 0;
    
    /**
     * Attempt to move a charge to this agent. If the charge is accepted then
     * this agent will store that charge in its future state, otherwise the
     * attempted transfer failed and the charge will not be stored.
     * @return true if accepted, false if not.
     */
    virtual bool acceptCharge(int charge) = 0;
    
    /**
     * Returns the charge of this node.
     */
    virtual int charge() = 0;

    virtual int fCharge() { return -1000; }
    
    /**
     * Perform a transport attempt
     */
    virtual Agent* transport() = 0;
    
    /**
     * Move on to the next time step.
     */
    virtual void completeTick() = 0;
    
    /**
     * Return the site number - mainly for debugging purposes...
     */
    virtual unsigned int site() = 0;

  };

} // End namespace Langmuir

#endif
