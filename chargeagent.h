#ifndef CHARGEAGENT_H
#define CHARGEAGENT_H

#include "agent.h"
#include <vector>

namespace Langmuir
{

  /**
    *  @class ChargeAgent
    *  @brief A agent representing a charge carrier.
    *
    *  Charge carriers that exist in the simulation are represented as charge agents.
    *  @date 06/07/2010
    */
  class ChargeAgent : public Agent
  {

  public:
    /**
     * @brief Default Constructor.
     *
     * Creates a charge agent.
     * @param world address of the world object.
     * @param site serial site index.
     */
    ChargeAgent(World *world, unsigned int site);

    /**
     * @brief virutal Destructor.
     */
    virtual ~ChargeAgent();

    /**
     * @brief charge.
     *
     * Data access to this charge carriers charge in units of electronic charge.
     * @return charge charge in units of the elementary charge.
     */
    virtual int charge();

    /**
     * @brief transport the carrier.
     *
     * Agent chooses a site and hands off to site.
     * @return site index of the site the charge carrier was moved to.  -1 if the transport was unsucessful.
     */
    virtual unsigned int transport();

    /**
     * @brief complete the tick.
     *
     * Update the simulation by accepting or rejecting a Monte Carlor move.
     */
    virtual void completeTick();

    /**
     * @brief charge removal.
     *
     * Data access to charge status.  Has the charge been removed from the system?
     * @return flag boolean removed status.
     */
    bool removed();

  protected:

    /**
     * @brief carrier charge.
     *
     * The charge of this charge carrier in units of elementary charge.
     */
    int m_charge;

    /**
     * @brief neighborlist.
     *
     * A list of this cells neighbors.
     */
    std::vector<unsigned int> m_neighbors;

    /**
     * @brief removed status.
     *
     * Carrier status. If or not the charge has been removed from the simulation.
     */
    bool m_removed;

    /**
     * @brief coulomb interaction.
     *
     * Calculate the potential difference arising from the Coulomb interaction
     * between the two proposed sites.
     * @param site serial cell index of site to interact with.
     * @return potential potential difference calculated.
     */
    double coulombInteraction(unsigned int newSite);

    /**
     * @brief defect interaction.
     *
     * Calculate the potential difference arising from the Coulomb interaction
     * between the carrier and all charged defect.
     * @param site serial cell index of site to interact with.
     * @return potential potential difference calculated.
     */
    double defectsCharged(unsigned int newSite);

    /**
     * @brief defect interaction.
     *
     * Calculate the potential difference arising from the Coulomb interaction
     * between the carrier and all charged traps.
     * @param site serial cell index of site to interact with.
     * @return potential potential difference calculated.
     */
    double trapsCharged(unsigned int newSite);

    /**
     * @brief coupling constant.
     *
     * Access the coupling constant for two sites.
     * @param type1 type of site 1.
     * @param type2 type of site 2.
     * @return coupling coupling constant between these two site types.
     */	  
    double couplingConstant(short id1, short id2);

    /**
     * @brief Transport attempt success.
     *
     * Apply the metropolis criterion for a transport attempt.
     *
     * @param pd potential difference between old and new site.
     * @param coupling coupling constant between old and new site types.
     * @param T temperature to calculate metropolis criterion at.
     * @return success Boolean.  True if transported, False if rejected.
     */
    bool attemptTransport(double pd, double coupling, double T);
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
