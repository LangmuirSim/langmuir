#ifndef CHARGEAGENT_H
#define CHARGEAGENT_H

#include "agent.h"

#include <vector>

namespace Langmuir
{

  class ChargeAgent : public Agent
  {

  public:
    ChargeAgent(World *world, unsigned int site);
    virtual ~ChargeAgent();

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
    virtual void completeTick();

    /**
     * Has the charge been removed from the system?
     */
    bool removed();

  protected:
    int m_charge;
    std::vector<unsigned int> m_neighbors;
    bool m_removed;

    /**
     * Get the coupling constant for the proposed move.
     */
    double couplingConstant(short id1, short id2);

    /**
     * Was the transport attempt successful.
     */
    bool attemptTransport(double pd, double coupling);
  };

  inline int ChargeAgent::charge()
  {
    return m_charge;
  }

  inline bool ChargeAgent::removed()
  {
    return m_removed;
  }

} // End namespace Langmuir

#endif // CHARGEAGENT_H
