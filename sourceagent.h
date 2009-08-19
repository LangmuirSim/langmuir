/*
 *  SourceAgent - agent to act as a source of charges
 */

#ifndef SOURCEAGENT_H
#define SOURCEAGENT_H

#include "agent.h"

#include <vector>

namespace Langmuir
{

  class SourceAgent : public Agent
  {

  public:
    SourceAgent(World *world, unsigned int site, double potential = 0.0);
    virtual ~SourceAgent();

    /**
     * Set the potential of the source.
     */
    virtual void setPotential(double potential)	{ m_potential = potential; }

    /**
     * Get the potential of this agent...
     */
    virtual double potential() { return m_potential; }

    /**
     * Returns the charge of this node.
     */
    virtual int charge();

    /**
     * Perform a transport attempt.
     */
    virtual unsigned int transport();

    /**
     * Set the maximum number of charges that can be present in the system.
     */
    void setMaxCharges(int number);

    /**
     * @return The maximum number of charges in the system.
     */
    int maxCharges();

    /**
     * Used to inform the source agent that a charge has been removed from the
     * system, allowing average charge density to be kept constant.
     */
    void decrementCharge();

    /**
     * Used to inform the source agent that a charge has been added to the
     * system, allowing average charge density to be kept constant.
     */
    void incrementCharge();

  private:
    double m_potential;
    double m_pBarrier;
    int m_charges; // Number of charges injected
    int m_maxCharges; // Maximum number of charges
  };

  inline void SourceAgent::setMaxCharges(int number)
  {
    m_maxCharges = number;
  }

  inline int SourceAgent::maxCharges()
  {
    return m_maxCharges;
  }

  inline void SourceAgent::decrementCharge()
  {
    if (m_charges > 0)
      --m_charges;
  }

  inline void SourceAgent::incrementCharge()
  {
    ++m_charges;
  }

} // End namespace Langmuir

#endif
