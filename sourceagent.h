#ifndef SOURCEAGENT_H
#define SOURCEAGENT_H

#include "agent.h"

namespace Langmuir
{

  class SourceAgent:public Agent
  {

  public:
  /**
    *  @brief A agent acting as a source for charges.
    *
    *  Depending on the potential of the source, positive or negative charges enter the simulation be emerging from the source.
    *  @date 06/07/2010
    */
    SourceAgent ( World * world, int site );

    /**
     * @brief virutal Destructor.
     */
    virtual ~ SourceAgent ();

    /**
     * @brief transport (charge injection) the source.
     *
     * Agent chooses a site and hands off to site.
     * @return site index of the site the charge carrier was moved to.  -1 if the transport was unsucessful.
     */
    virtual int transport ( );

    /**
      * @bried charge of the source (cause the makes sense!)
      *
      * The Source doesnt really have a charge.  But this method is pure within the base agent class.
      */
    virtual int charge() { return 1; }

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
    int maxCharges();

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

  protected:

    /**
     * @brief site interaction.
     *
     * Calculate the site potential at the proposed site.
     * @param site serial cell index of site to interact with.
     * @return potential potential calculated.
     */
    double siteInteraction(int newSite);

    /**
     * @brief coulomb interaction.
     *
     * Calculate the potential arising from the Coulomb interaction
     * between the proposed site and other carriers.
     * @param site serial cell index of site to interact with.
     * @return potential potential calculated.
     */
    double coulombInteraction(int newSite);

    /**
     * @brief coulomb interaction.
     *
     * Calculate the potential difference arising from the Coulomb interaction
     * between the proposed site and images of other carriers.
     * @param site serial cell index of site to interact with.
     * @return potential potential calculated.
     */
    double imageInteraction(int newSite);

    /**
     * @brief Transport attempt success.
     *
     * Apply the metropolis criterion for a transport attempt.
     *
     * @param pd potential difference between old and new site.
     * @return success Boolean.  True if transported, False if rejected.
     */
    bool attemptTransport(double pd);

  private:

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
