#ifndef DRAINAGENT_H
#define DRAINAGENT_H

#include "agent.h"

namespace Langmuir
{
  class ChargeAgent;
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
    DrainAgent(Agent::Type type, World *world, int site);

    /**
     * @brief virutal Destructor.
     */
   ~DrainAgent();

    /**
     * @brief increment charge counters
     *
     * Accept charge carrier
     * @param type is hole or electron
     */
    virtual void acceptCharge(Agent::Type type);

    /**
     * @brief attept charge move.
     *
     * Try to accept a charge carrier
     * @param charge is hole or electron
     */
    virtual bool attemptTransport(ChargeAgent *charge);

    /**
     * @brief total charges.
     *
     * Total number of electrons the drain has accepted.
     */
    unsigned long acceptedElectrons();

    /**
     * @brief total charges.
     *
     * Total number of holes the drain has accepted.
     */
    unsigned long acceptedHoles();

  private:

    /**
      * @brief total charges.
      *
      * Total number of charges accepted by the drain.
      */
    unsigned long m_acceptedElectrons; // Counter - number of accepted electrons

    /**
      * @brief total charges.
      *
      * Total number of charges accepted by the drain.
      */
    unsigned long m_acceptedHoles; // Counter - number of accepted electrons
  };

  inline unsigned long DrainAgent::acceptedElectrons()
  {
    return m_acceptedElectrons;
  }

  inline unsigned long DrainAgent::acceptedHoles()
  {
    return m_acceptedHoles;
  }
} // End namespace Langmuir

#endif
