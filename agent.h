#ifndef AGENT_H
#define AGENT_H

#include <QTextStream>
#include <QMetaObject>
#include <QMetaEnum>
#include <QVector>
#include <QObject>
#include <QString>
#include <QDebug>

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
private:
    Q_OBJECT
    Q_DISABLE_COPY(Agent)
    Q_ENUMS(Type)

public:
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
    static QString toQString(const Agent::Type e);

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

inline QString Agent::toQString(const Agent::Type e)
{
    const QMetaObject &QMO = Agent::staticMetaObject;
    QMetaEnum QME = QMO.enumerator(QMO.indexOfEnumerator("Type"));
    return QString("%1").arg(QME.valueToKey(e));
}

inline QTextStream &operator<<(QTextStream &stream,const Agent::Type e)
{
    return stream << Agent::toQString(e);
}

inline QDebug operator<<(QDebug dbg,const Agent::Type e)
{
    return dbg << qPrintable(Agent::toQString(e));
}

}

#endif
