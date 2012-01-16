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
class Agent : public QObject
{
Q_OBJECT public:
    /**
     * @enum Different Agent types
     *
     */
    // The numbers are used as an index to access a matrix of coupling constants - so be careful changing things
    enum Type
    {
        Empty         =   0,
        Electron      =   1,
        Hole          =   2,
        Defect        =   3,
        Source        =   4,
        Drain         =   5,
        SIZE          =   6
    };
    static QString typeToQString(const Agent::Type &type);

    /**
     * @brief Default Constructor.
     *
     * Creates a agent.
     * Assigns the agent a serial site index.
     * Points the agent to the world, where simulation parameters may be accessed.
     * @param world address of the world object.
     * @param site serial site index.
     */
    Agent(Type type, World *world = 0, int site = 0, QObject *parent = 0);

    /**
     * @brief virutal Destructor.
     */
    virtual ~Agent() { }

    /**
     * @brief set neighbors.
     *
     * Set the nearest neighbours of the agent.
     */
    void setNeighbors(QVector<int> neighbors);

    /**
     * @brief set neighbors.
     *
     * Get the nearest neighbours of the agent.
     */
    const QVector<int>& getNeighbors() const;

    /**
     * @brief Set site.
     *
     * Set the site number
     */
    void setSite(int site, bool future = false);

    /**
     * @brief Get site.
     *
     * Return the site number
     */
    int site(bool future = false);

    /**
     * @brief Get type.
     *
     * Return the type number
     */
    Type type() const;

    /**
     * @brief print out agent parameters
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

inline Agent::Agent(Type type, World *world, int site, QObject *parent) : QObject(parent),
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

inline QDebug operator<<(QDebug dbg, const Agent::Type &type)
{
    return dbg << qPrintable(Agent::typeToQString(type));
}

inline QString Agent::typeToQString(const Agent::Type &type)
{
    switch(type)
    {
    case Agent::Empty:
    {
        return QString("Empty");
        break;
    }

    case Agent::Electron:
    {
        return QString("Electron");
        break;
    }

    case Agent::Hole:
    {
        return QString("Hole");
        break;
    }

    case Agent::Defect:
    {
        return QString("Defect");
        break;
    }

    case Agent::Source:
    {
        return QString("Source");
        break;
    }

    case Agent::Drain:
    {
        return QString("Drain");
        break;
    }

    case Agent::SIZE:
    {
        return QString("SIZE");
        break;
    }

    default:
    {
        return QString("UnknownType");
        break;
    }
    }
    return QString("UnknownType");
}

inline QDebug operator<<(QDebug dbg, const Agent& agent)
{
    return dbg << qPrintable(agent.toQString());
}

inline QString Agent::toQString() const
{
    QString result;
    QTextStream stream(&result);

    int w = 20;
    stream << QString("%1 %2 %3 %4 %5")
              .arg(                      "type:", w )
              .arg(                           "", w )
              .arg(                           "", w )
              .arg( Agent::typeToQString(m_type), w )
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
              .arg('\n');

    if ( m_neighbors.size() == 0 )
    {
        stream << QString("%1 %2 %3 %4 %5")
                  .arg(      "neighbors:", w        )
                  .arg(               "-", w        )
                  .arg(               "-", w        )
                  .arg(               "-", w        )
                  .arg('\n');
    }
    for ( int i = 0; i < m_neighbors.size(); i+=3 )
    {
        QString a = "-";
        QString b = "-";
        QString c = "-";
        QString n = ":";
        if ( i == 0 ) n = "neighbors:";
        if ( i + 0 < m_neighbors.size() ) { a = QString("%1").arg(m_neighbors[i+0]); }
        if ( i + 1 < m_neighbors.size() ) { b = QString("%1").arg(m_neighbors[i+1]); }
        if ( i + 2 < m_neighbors.size() ) { c = QString("%1").arg(m_neighbors[i+2]); }
        stream << QString("%1 %2 %3 %4 %5")
                  .arg(                 n, w        )
                  .arg(                 a, w        )
                  .arg(                 b, w        )
                  .arg(                 c, w        )
                  .arg('\n',1);
    }
    return result;
}
}

#endif
