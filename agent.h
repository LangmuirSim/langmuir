#ifndef AGENT_H
#define AGENT_H

#include <vector>
#include <QtCore>

namespace Langmuir{

  class World;

  /**
    *  @class Agent
    *  @brief Virtual Base Class.
    *
    *  Pure virtual base class to inherited by all agent classes
    *  @date 06/07/2010
    */
  class Agent {

  public:
    /**
     * @enum Different Agent types
     */
    enum Type {
      Source,
      Drain,
      Charge
    };

    /**
     * @brief Default Constructor.
     *
     * Creates a agent.
     * Assigns the agent a serial site index.
     * Points the agent to the world, where simulation parameters may be accessed.
     * @param world address of the world object.
     * @param site serial site index.
     */
    Agent(Type type, World *world = 0, unsigned int site = 0);

    /**
     * @brief virutal Destructor.
     */
    virtual ~Agent() { }

    /**
     * @brief set neighbors.
     *
     * Set the nearest neighbours of the agent.
     */
    virtual void setNeighbors(std::vector<unsigned int> neighbors);

    /**
     * @brief charge.
     *
     * Data access to this charge carriers charge in units of electronic charge.
     * @return charge charge in units of the elementary charge.
     */
    virtual int charge() = 0;

    /**
     * @brief transport the carrier.
     *
     * Agent chooses a site and hands off to site.
     * @return site index of the site the charge carrier was moved to.  -1 if the transport was unsucessful.
     */
    virtual unsigned int transport() = 0;

    /**
     * @brief complete the tick.
     *
     * Update the simulation by accepting or rejecting a Monte Carlor move.
     */
    virtual void completeTick();

    /**
     * @brief Set site.
     *
     * Set the site number
     */
    virtual void setSite(unsigned int site, bool future = false);

    /**
     * @brief Get site.
     *
     * Return the site number
     */
    virtual unsigned int site(bool future = false);

  protected:
    /**
      * @brief current site.
      *
      * Serial site index for the current site.
      */
    unsigned int m_site;

    /**
      * @brief future site.
      *
      * Serial site index for the future site.
      */
    unsigned int m_fSite;

    /**
      * @brief Simulation world.
      *
      * Address to the world object.
      */
    World *m_world;

    /**
      * @brief neighbor cells.
      *
      * List of neighboring sites in the grid.
      */
    std::vector<unsigned int> m_neighbors;

    /**
      * @brief Agent type.
      *
      * Number identifying the agent type. 0 = charge carrier, 1 = defect, 2 = source, 3 = drain.
      */
    short m_type;
  };

  inline Agent::Agent(Type type, World *world, unsigned int site) :
        m_site(-1), m_fSite(site), m_world(world), m_type(type)
  {
  }

  inline void Agent::setNeighbors(std::vector<unsigned int> neighbors)
  {
    m_neighbors = neighbors;
  }

  inline void Agent::completeTick()
  {
    m_site = m_fSite;
  }

  inline void Agent::setSite(unsigned int site, bool future)
  {
    if (future) m_fSite = site;
    else m_site = site;
  }

  inline unsigned int Agent::site(bool future)
  {
    if (future) return m_fSite;
    else return m_site;
  }

} // End namespace Langmuir

#endif
