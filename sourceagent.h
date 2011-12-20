#ifndef SOURCEAGENT_H
#define SOURCEAGENT_H

#include "agent.h"

namespace Langmuir
{
  /**
    *  @class SourceAgent
    *  @brief For injectiing charges
    */
  class SourceAgent:public Agent
  {

  public:
   /**
    *  @brief A agent acting as a source for charges.
    *
    *  Depending on the potential of the source, positive or negative charges enter the simulation be emerging from the source.
    *  @date 06/07/2010
    */
    SourceAgent(Agent::Type type, World * world, int site);

    /**
     * @brief virutal Destructor.
     */
    virtual ~SourceAgent();

    /**
     * @brief max charges.
     * 
     * Get the max number of charges possible.
     */
    int maxCharges();

    /**
     * @brief max electrons.
     *
     * Get the max number of electrons possible.
     */
    int maxElectrons();

    /**
     * @brief max holes.
     *
     * Get the max number of holes possible.
     */
    int maxHoles();

    /**
     * @brief max charges.
     *
     * Get the number of carriers.
     */
    int carrierCount();

    /**
     * @brief max electrons.
     *
     * Get the number of electrons.
     */
    int electronCount();

    /**
     * @brief max holes.
     *
     * Get the number of holes.
     */
    int holeCount();

    void shouldInject(Agent::Type type, int site);
    void injectHoleAtSite(int site);
    void injectElectronAtSite(int site);
    bool validToInjectHoleAtSite(int site);
    bool validToInjectElectronAtSite(int site);
    bool seedHole();
    bool seedElectron();
    int randomSiteID();

  private:

    /**
      * @brief Attempted injections
      *
      * Number of times this source tried to inject charges
      */
    unsigned int m_attempts;

    /**
      * @brief Successful injections
      *
      * Number of times this source injected charges
      */
    unsigned int m_successes;

    /**
      * @brief Maximum number of carriers
      *
      * Number of electrons allowed
      */
    static int m_maxElectrons;

    /**
      * @brief Maximum number of carriers
      *
      * Number of holes allowed
      */
    static int m_maxHoles;

  };

}                                // End namespace Langmuir

#endif
