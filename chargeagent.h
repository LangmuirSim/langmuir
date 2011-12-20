#ifndef CHARGEAGENT_H
#define CHARGEAGENT_H

#include "agent.h"

namespace Langmuir
{

  class Grid;
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
    ChargeAgent(Agent::Type type, World *world, int site);

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
    virtual void setCharge( int value );

    /**
     * @brief charge.
     *
     * Data access to this charge carriers charge in units of electronic charge.
     * @return charge charge in units of the elementary charge.
     */
    virtual int charge();

    /**
      * @brief choose future site
      */
    virtual void chooseFuture();

    /**
      * @brief decide if future chosen site is ok
      */
    virtual void decideFuture();

    /**
     * @brief complete the tick.
     *
     * Update the simulation by accepting or rejecting a Monte Carlo move.
     */
    virtual void completeTick();

    /**
     * @brief charge removal.
     *
     * Data access to charge status.  Has the charge been removed from the system?
     * @return flag boolean removed status.
     */
    bool removed();

    /**
     * @brief lifetime or step count.
     *
     * how many steps the charge agent has been in existance
     * @return steps.
     */
    int lifetime();

    /**
     * @brief distance traveled during lifetime.
     *
     * how far has the charge traveled during its lifetime
     * @return distance.
     */
    double distanceTraveled();

    /**
      * @brief set OpenCLID
      */
    void setOpenCLID( int id );

    /**
      * @brief get OpenCLID
      */
    int getOpenCLID();

    /**
     * @brief coulomb interaction.
     *
     * Calculate the potential difference arising from the Coulomb interaction
     * between the two proposed sites.
     * @param site serial cell index of site to interact with.
     * @return potential potential difference calculated.
     */
    double coulombInteraction(int newSite);

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
    QVector<int> m_neighbors;

    /**
     * @brief removed status.
     *
     * Carrier status. If or not the charge has been removed from the simulation.
     */
    bool m_removed;

   /**
     * @brief steps in existance
     *
     * lifetime of the carrier in steps
     */
    int m_lifetime;

   /**
     * @brief distance traveled
     *
     * distance the carrier has traveled during its lifetime
     */
    double m_distanceTraveled;

   /**
     *  @brief grid pointer
     *
     *  points to holeGrid or electronGrid depending upon what grid the carrier is in
     */
    Grid *m_grid;

   /**
     * @brief Host memory array index ~ constantly changing...
     * this is an index used to read and write information
     * about this charge carrier to and from a global array(s).
     * In particular, its used to write site IDS to m_iHost, m_fHost,
     * and read Coulomb energies from m_oHost. ( all OpenCL stuff )
     */
    int m_openClID;
  };

  inline int ChargeAgent::charge()
  {
    return m_charge;
  }

  inline void ChargeAgent::setCharge( int value )
  {
    m_charge = value;
  }

  inline bool ChargeAgent::removed()
  {
    return m_removed;
  }

  inline int ChargeAgent::lifetime()
  {
   return m_lifetime;
  }

  inline double ChargeAgent::distanceTraveled()
  {
   return m_distanceTraveled;
  }

  inline void ChargeAgent::setOpenCLID( int id )
  {
      m_openClID = id;
  }

  inline int ChargeAgent::getOpenCLID( )
  {
      return m_openClID;
  }

} // End namespace Langmuir

#endif // CHARGEAGENT_H
