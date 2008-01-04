/*
 *  HoppingAgent - agent performing hopping...
 *
 */

#ifndef __HOPPINGAGENT_H
#define __HOPPINGAGENT_H

#include <vector>

namespace Langmuir{
  
  class HoppingAgent {
    
  public:
    HoppingAgent();
    ~HoppingAgent();
    
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
    
  };
  
#endif
