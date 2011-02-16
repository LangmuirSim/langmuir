#ifndef SOURCEAGENT_H
#define SOURCEAGENT_H

#include "agent.h"

namespace Langmuir
{

  class SourceAgent:public Agent
  {

  public:
  /**
    *  @class DrainAgent
    *  @brief A agent acting as a source for charges.
    *
    *  Depending on the potential of the source, positive or negative charges enter the simulation be emerging from the source.
    *  @date 06/07/2010
    */
    SourceAgent (World * world, unsigned int site, double potential =0.0, double barrier = 0.10, int barrierCalculationType = 0,
     bool chargedDefects = false, bool chargedTraps = false, double temperatureKelvin = 300.0, int zDefect = -1, int zTrap = -1 );

    /**
     * @brief virutal Destructor.
     */
    virtual ~ SourceAgent ();

    /**
     * @brief set potential.
     *
     * Set the potential of the source.
     */
    virtual void setPotential (double potential)
    {
      m_potential = potential;
    }

    /**
     * @brief get potential.
     *
     * Get the potential of the source.
     */
    virtual double potential ()
    {
      return m_potential;
    }

    /**
     * @brief charge of agent.
     *
     * Get the charge of this agent.  What does that even mean for a source?
     * @return charge the charge of this agent.
     */
    virtual int charge ();

    /**
     * @brief transport (from???) the source. 
     *
     * Agent chooses a site and hands off to site.
     * @return site index of the site the charge carrier was moved to.  -1 if the transport was unsucessful.
     */
    virtual unsigned int transport ();

    /**
     * @brief max charges.
     *
     * Set the maximum number of charges that can be present in the system.
     */
    void setMaxCharges (int number);

    /**
     * @brief max charges.
     * 
     * Get the max number of possible charges.
     * @return The maximum number of charges in the system.
     */
    int maxCharges ();

    /**
     * @brief remove a charge.
     *
     * Used to inform the source agent that a charge has been removed from the
     * system, allowing average charge density to be kept constant.
     */
    void decrementCharge ();

    /**
     * @brief add a charge.
     *
     * Used to inform the source agent that a charge has been added to the
     * system, allowing average charge density to be kept constant.
     */
    void incrementCharge ();

  private:
    /**
      * @brief potential.
      *
      * Potential of the source.
      */
    double m_potential;

    /**
      * @brief Barrier.
      *
      * Not sure what this is.
      */
    double m_pBarrier;

    /**
      * @brief Injection Barrier Calculation
      *
      * What type of barrier calculation to perform.
      * (0) constant: compare to a set probability
      * (1) coulomb: determine probability from a Coulomb loop
      */
    int m_pBarrierCalculationType;

    /**
     * @brief defect status.
     *
     * If or not charged defects should be calculated.
     */
    bool m_chargedDefects; // include charged defects?

    /**
     * @brief trap status.
     *
     * If or not charged traps should be calculated.
     */
    bool m_chargedTraps; // include charged traps?

    /**
     * @brief simulation temperature.
     *
     * Temperature to compute the metropolis criterion at.
     * @param site serial cell index of site to interact with.
     */
    double m_temperatureKelvin;

    /**
     * @brief Charge Defect charge.
     *
     * The charge on defects in the simulation.
     */
    int m_zDefect;

    /**
     * @brief Charge Defect charge.
     *
     * The charge on traps in the simulation.
     */
    int m_zTrap;

    /**
      * @brief Charges.
      *
      * Number of charges injected.
      */
    int m_charges;

    /**
      * @brief Max Charges.
      *
      * Maximum number of charges.
      */
    int m_maxCharges;

  };

  inline void SourceAgent::setMaxCharges (int number)
  {
    m_maxCharges = number;
  }

  inline int SourceAgent::maxCharges ()
  {
    return m_maxCharges;
  }

  inline void SourceAgent::decrementCharge ()
  {
    if (m_charges > 0)
      --m_charges;
  }

  inline void SourceAgent::incrementCharge ()
  {
    ++m_charges;
  }

}                                // End namespace Langmuir

#endif
