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
    DrainAgent(World *world, unsigned int site, double potential = 0.0);
    ~DrainAgent();

    /**
     * Set the potential of the source.
     */
    virtual void setPotential(double potential) { m_potential = potential; }

    /**
     * Get the potential of this agent...
     */
    virtual double potential() { return m_potential; }

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
     * Total number of charges the drain has accepted.
     */
    unsigned long acceptedCharges();

  private:
    double m_potential;
    unsigned long m_acceptedCharges; // Counter - number of accepted charges
  };

  inline unsigned long DrainAgent::acceptedCharges()
  {
    return m_acceptedCharges;
  }

} // End namespace Langmuir

#endif
