/*
 *  HoppingAgent - agent performing hopping...
 *
 */

#ifndef __HOPPINGAGENT_H
#define __HOPPINGAGENT_H

#include "agent.h"

#include <vector>

namespace Langmuir{
  
  class HoppingAgent : public Agent
  {
    
  public:
    HoppingAgent();
    ~HoppingAgent();
    
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
    virtual bool transport();

  private:
    std::vector<Agent *> *m_neighbors;
    int m_charge;
    
  };

} // End namespace Langmuir

#endif
