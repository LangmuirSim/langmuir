/*
 *  Agent - pure virtual base class to inherited by all agent classes
 *
 */

#ifndef AGENT_H
#define AGENT_H

#include <vector>

namespace Langmuir{

  class Agent {

  public:
    Agent (unsigned int site) { m_site = site; }
    virtual ~Agent() { }

    /**
     * Set the nearest neighbours of the agent.
     */
    virtual void setNeighbors(std::vector<Agent *> neighbors) = 0;

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
     * The potential barrier to making a move.
     */
    virtual double pBarrier() { return 0.0; }

    virtual void setPBarrier(double) { ; }

    /**
     * Perform a transport attempt
     */
    virtual Agent* transport() = 0;

    /**
     * Move on to the next time step.
     */
    virtual void completeTick() = 0;

    /**
     * Set the site number
     */
    virtual void setSite(unsigned int site, bool future = false)
    {
      if (future) m_fSite = site;
      else m_site = site;
    }

    /**
     * Return the site number
     */
    virtual unsigned int site(bool future)
    {
      if (future) return m_fSite;
      else return m_site;
    }

  protected:
    unsigned int m_site;
    unsigned int m_fSite;

  };

} // End namespace Langmuir

#endif
