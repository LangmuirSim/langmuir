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

  protected:
    int m_charge;
  };


  inline int ChargeAgent::charge()
  {
    return m_charge;
  }

} // End namespace Langmuir

#endif // CHARGEAGENT_H
