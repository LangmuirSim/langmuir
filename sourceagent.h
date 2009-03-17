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

  private:
    double m_potential;
    double m_pBarrier;
    int m_charges; // Number of charges injected
  };

} // End namespace Langmuir

#endif
