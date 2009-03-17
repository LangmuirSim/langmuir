/*
 *  Agent - pure virtual base class to inherited by all agent classes
 *
 */

#ifndef AGENT_H
#define AGENT_H

#include <vector>

namespace Langmuir{

  class World;

  class Agent {

  public:
    Agent(World *world = 0, unsigned int site = 0);
    virtual ~Agent() { }

    /**
     * Set the nearest neighbours of the agent.
     */
    virtual void setNeighbors(std::vector<unsigned int> neighbors);

    /**
     * Returns the charge of this node.
     */
    virtual int charge() = 0;

    /**
     * Perform a transport attempt - agent chooses a site and hands off to site
     */
    virtual unsigned int transport() = 0;

    /**
     * Move on to the next time step.
     */
    virtual void completeTick();

    /**
     * Set the site number
     */
    virtual void setSite(unsigned int site, bool future = false);

    /**
     * Return the site number
     */
    virtual unsigned int site(bool future = false);

  protected:
    unsigned int m_site, m_fSite; // Current and future site index
    World *m_world;
    std::vector<unsigned int> m_neighbors;

  };

  inline Agent::Agent (World *world, unsigned int site) :
        m_site(-1), m_fSite(site), m_world(world)
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
