#ifndef AGENT_H
#define AGENT_H

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
     *
     */
    // The numbers are used as an index to access a matrix of coupling constants - so be careful changing things
    enum Type {
      Empty = 0,
      Electron = 1,
      Hole = 2,
      Defect = 3,
      HoleSource = 4,
      ElectronSource = 5,
      ExcitonSource = 6,
      DrainL = 7,
      DrainR = 8,
      SIZE = 9
    };
    static QString typeToQString(const Agent::Type &t);

    /**
     * @brief Default Constructor.
     *
     * Creates a agent.
     * Assigns the agent a serial site index.
     * Points the agent to the world, where simulation parameters may be accessed.
     * @param world address of the world object.
     * @param site serial site index.
     */
    Agent(Type type, World *world = 0, int site = 0);

    /**
     * @brief virutal Destructor.
     */
    virtual ~Agent() { }

    /**
     * @brief set neighbors.
     *
     * Set the nearest neighbours of the agent.
     */
    virtual void setNeighbors(QVector<int> neighbors);

    /**
     * @brief set neighbors.
     *
     * Get the nearest neighbours of the agent.
     */
    virtual const QVector<int>& getNeighbors() const;

    /**
     * @brief Set site.
     *
     * Set the site number
     */
    virtual void setSite(int site, bool future = false);

    /**
     * @brief Get site.
     *
     * Return the site number
     */
    virtual int site(bool future = false);

    /**
     * @brief Get type.
     *
     * Return the type number
     */
    virtual Type type() const;

  protected:
    /**
      * @brief current site.
      *
      * Serial site index for the current site.
      */
    int m_site;

    /**
      * @brief future site.
      *
      * Serial site index for the future site.
      */
    int m_fSite;

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
    QVector<int> m_neighbors;

    /**
      * @brief Agent type.
      *
      * Number identifying the agent type. 0 = charge carrier, 1 = defect, 2 = source, 3 = drain.
      */
    Type m_type;
  };

  inline Agent::Agent(Type type, World *world, int site) :
        m_site(-1), m_fSite(site), m_world(world), m_type(type)
  {
  }

  inline void Agent::setNeighbors(QVector<int> neighbors)
  {
    m_neighbors = neighbors;
  }

  inline const QVector<int>& Agent::getNeighbors() const
  {
    return m_neighbors;
  }

  inline void Agent::setSite(int site, bool future)
  {
    if (future) m_fSite = site;
    else m_site = site;
  }

  inline int Agent::site(bool future)
  {
    if (future) return m_fSite;
    else return m_site;
  }

  inline Agent::Type Agent::type() const
  {
    return m_type;
  }

  inline QDebug operator<<(QDebug dbg, const Agent::Type &t)
  {
      return dbg << qPrintable(Agent::typeToQString(t));
  }

  inline QString Agent::typeToQString(const Agent::Type &t)
  {
      switch(t)
      {
          case Agent::Empty:
          {
              return QString("Agent::Type(%1):Empty").arg(int(t));
              break;
          }

          case Agent::Hole:
          {
              return QString("Agent::Type(%1):Hole").arg(int(t));
              break;
          }

          case Agent::Electron:
          {
              return QString("Agent::Type(%1):Electron").arg(int(t));
              break;
          }

          case Agent::Defect:
          {
              return QString("Agent::Type(%1):Defect").arg(int(t));
              break;
          }

          case Agent::HoleSource:
          {
              return QString("Agent::Type(%1):SourceL").arg(int(t));
              break;
          }

          case Agent::ElectronSource:
          {
              return QString("Agent::Type(%1):SourceR").arg(int(t));
              break;
          }

          case Agent::DrainL:
          {
              return QString("Agent::Type(%1):DrainL").arg(int(t));
              break;
          }

          case Agent::DrainR:
          {
              return QString("Agent::Type(%1):DrainR").arg(int(t));
              break;
          }

          default:
          {
              return QString("Agent::Type(%1):UnknownType").arg(int(t));
              break;
          }
      }
      return QString("Agent::Type(?):UnknownType");
  }

} // End namespace Langmuir

#endif
