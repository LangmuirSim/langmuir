#ifndef DRAINAGENT_H
#define DRAINAGENT_H

#include "agent.h"

#include <vector>

namespace Langmuir
{

  /**
    *  @class DrainAgent
    *  @brief A agent acting as a sink for charges.
    *
    *  Depending on the potential of the drain, positive or negative charges leave the simulation be traveling to the drain.
    *  @date 06/07/2010
    */
  class DrainAgent : public Agent
  {

  public:
    /**
     * @brief Default Constructor.
     *
     * Creates a charge agent.
     * Assigns the agent a serial site index.
     * Points the agent to the world, where simulation parameters may be accessed.
     * Sets the potential of the drain agent.
     * @param world address of the world object.
     * @param site serial site index.
     * @param potential potential of the drain.
     * @warning default potential is set to zero.
     */
    DrainAgent(World *world, unsigned int site, double potential = 0.0);

    /**
     * @brief virutal Destructor.
     */
   ~DrainAgent();

    /**
     * @brief set potential.
     *
     * Set the potential of the drain.
     */
    virtual void setPotential(double potential) { m_potential = potential; }

    /**
     * @brief get potential.
     *
     * Get the potential of the drain.
     */
    virtual double potential() { return m_potential; }

    /**
     * @brief attept charge move.
     *
     * Attempt to move a charge to this agent. If the charge is accepted then
     * this agent will store that charge in its future state, otherwise the
     * attempted transfer failed and the charge will not be stored.
     * @param charge charge value in elementary units.
     * @return true if accepted, false if not.
     */
    virtual bool acceptCharge(int charge);

    /**
     * @brief charge of agent.
     *
     * Get the charge of this agent.  What does that even mean for a drain?
     * @return charge the charge of this agent.
     */
    virtual int charge();

    /**
     * @brief transport the drain. What does that even mean?
     *
     * Agent chooses a site and hands off to site.
     * @return site index of the site the charge carrier was moved to.  -1 if the transport was unsucessful.
     */
    virtual unsigned int transport();

    /**
     * @brief total charges.
     *
     * Total number of charges the drain has accepted.
     * @return count the number of charges accepted.
     */
    unsigned long acceptedCharges();

  private:
    /**
      * @brief potential.
      *
      * Potential of the drain.
      */
    double m_potential;

    /**
      * @brief total charges.
      *
      * Total number of charges accepted by the drain.
      */
    unsigned long m_acceptedCharges; // Counter - number of accepted charges
  };

  inline unsigned long DrainAgent::acceptedCharges()
  {
    return m_acceptedCharges;
  }

} // End namespace Langmuir

#endif
