/*
 *  HoppingAgent - agent performing hopping...
 *
 */

#ifndef HOPPINGAGENT_H
#define HOPPINGAGENT_H

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
    virtual ~HoppingAgent();

    /**
     * Set the nearest neighbours of the agent.
     */
    virtual void setNeighbors(std::vector<Agent *> neighbors);

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

    virtual double pBarrier();

    virtual void setPBarrier(double pBarrier);

    /**
     * Perform a transport attempt
     */
    virtual Agent* transport();

    /**
     * Move on to the next time step.
     */
    virtual void completeTick();

  private:
    const Grid* m_grid;
    std::vector<Agent *> m_neighbors;
    int m_charge, m_fCharge;
    Rand *m_rand;
    double m_pBarrier;
  };

} // End namespace Langmuir

#endif
