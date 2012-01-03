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
      HoleSourceL = 4,
      HoleSourceR = 5,
      ElectronSourceL = 6,
      ElectronSourceR = 7,
      ExcitonSource = 8,
      HoleDrainL = 9,
      HoleDrainR = 10,
      ElectronDrainL = 11,
      ElectronDrainR = 12,
      SIZE = 13
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

    /**
      * @brief Human Readable
      */
    virtual QString toQString() const;

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

  inline QDebug operator<<(QDebug dbg, const Agent &agent)
  {
      return dbg << qPrintable(agent.toQString());
  }

  inline QString Agent::typeToQString(const Agent::Type &t)
  {
      switch(t)
      {
          case Agent::Empty:
          {
              return QString("Empty(%1)").arg(t);
              break;
          }

          case Agent::Hole:
          {
              return QString("Hole(%1)").arg(t);
              break;
          }

          case Agent::Electron:
          {
              return QString("Electron(%1)").arg(t);
              break;
          }

          case Agent::Defect:
          {
              return QString("Defect(%1)").arg(t);
              break;
          }

          case Agent::HoleSourceL:
          {
              return QString("HoleSourceL(%1)").arg(t);
              break;
          }

          case Agent::HoleSourceR:
          {
              return QString("HoleSourceR(%1)").arg(t);
              break;
          }

          case Agent::ElectronSourceL:
          {
              return QString("ElectronSourceL(%1)").arg(t);
              break;
          }

          case Agent::ElectronSourceR:
          {
              return QString("ElectronSourceR(%1)").arg(t);
              break;
          }

          case Agent::ExcitonSource:
          {
              return QString("ExcitonSource(%1)").arg(t);
              break;
          }

          case Agent::HoleDrainL:
          {
              return QString("HoleDrainL(%1)").arg(t);
              break;
          }

          case Agent::HoleDrainR:
          {
              return QString("HoleDrainR(%1)").arg(t);
              break;
          }

          case Agent::ElectronDrainL:
          {
              return QString("ElectronDrainL(%1)").arg(t);
              break;
          }

          case Agent::ElectronDrainR:
          {
              return QString("ElectronDrainR(%1)").arg(t);
              break;
          }

          default:
          {
              return QString("UnknownType(?)");
              break;
          }
      }
      return QString("UnknownType(?)");
  }

  inline QString Agent::toQString() const
  {
      QString result;
      QTextStream stream(&result);
      stream << "[ " << Agent::typeToQString(m_type) << "\n";

      int w = 20;
      stream << QString("%1 %2 %3 %4 %5")
                .arg(           "type:", w )
                .arg(                "", w )
                .arg(                "", w )
                .arg(            m_type, w )
                .arg('\n');

      stream << QString("%1 %2 %3 %4 %5")
                .arg(           "site:", w )
                .arg(                "", w )
                .arg(                "", w )
                .arg(            m_site, w )
                .arg('\n');

      stream << QString("%1 %2 %3 %4 %5")
                .arg(          "fSite:", w )
                .arg(                "", w )
                .arg(                "", w )
                .arg(           m_fSite, w )
                .arg('\n');

      QString address; QTextStream astream(&address); astream << this;
      stream << QString("%1 %2 %3 %4 %5")
                .arg(        "address:", w )
                .arg(                "", w )
                .arg(                "", w )
                .arg(           address, w )
                .arg(']',1);

      return result;
  }
} // End namespace Langmuir

#endif
