#ifndef AGENT_H
#define AGENT_H

#include <QTextStream>
#include <QMetaObject>
#include <QMetaEnum>
#include <QVector>
#include <QObject>
#include <QString>
#include <QDebug>

namespace LangmuirCore
{

class World;

//! A class that abstractly represents an object that can occupy grid sites
/*!
  Agents can be Electrons, Holes, Defects, Sources, or Drains.  The Agent class
  encodes basic information that all agents have, regardless of their type.
  For examples, all agents occupy a grid site, have knowledge of their
  neighboring sites, and know their own type.
 */
class Agent : public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(Agent)
    Q_ENUMS(Type)

public:
    //! An identifier for the type of Agent
    enum Type
    {
        //! Empty Grid site
        Empty         =   0,

        //! ElectronAgent
        Electron      =   1,

        //! HoleAgent
        Hole          =   2,

        //! Defective Grid site
        Defect        =   3,

        //! SourceAgent
        Source        =   4,

        //! DrainAgent
        Drain         =   5,

        //! Number of Agent Types
        SIZE          =   6
    };
    static QString toQString(const Agent::Type e);

public:
    //! Create an Agent
    /*!
      \param type identifier enum
        - for example: Electron, Hole, etc.
      \param world reference world object
      \param site grid site id Agent occupies
      \param parent parent QObject
     */
    Agent(Type type, World &world, int site = 0, QObject *parent = 0);

    //! Destroy Agent
    virtual ~Agent();

    //! Get Agent neighbor list
    const QVector<int>& getNeighbors() const;

    //! Set Agent neighbor list
    void setNeighbors(QVector<int> neighbors);

    //! Get Agent current site
    int getCurrentSite() const;

    //! Get Agent future site
    int getFutureSite() const;

    //! Set Agent current site
    void setCurrentSite(int site);

    //! Set Agent future site
    void setFutureSite(int site);

    //! Get Agent::Type enum
    Type getType() const;

    //! Get Langmuir::World reference
    World& getWorld() const;

protected:

    //! Current site the Agent occupies
    int m_site;

    //! Future site the Agent \b will occupy
    int m_fSite;

    //! Reference to World object
    World &m_world;

    //! List fo neighboring site ids
    QVector<int> m_neighbors;

    //! Agent Type enum
    Type m_type;
};

inline Agent::Agent(Type type, World &world, int site, QObject *parent) : QObject(parent),
    m_site(-1), m_fSite(site), m_world(world), m_type(type)
{
}

inline Agent::~Agent()
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

inline int Agent::getCurrentSite() const
{
    return m_site;
}

inline int Agent::getFutureSite() const
{
    return m_fSite;
}

inline void Agent::setCurrentSite(int site)
{
    m_site = site;
}

inline void Agent::setFutureSite(int site)
{
    m_fSite = site;
}

inline Agent::Type Agent::getType() const
{
    return m_type;
}

inline World& Agent::getWorld() const
{
    return m_world;
}

//! Convert Agent type enum to QString
inline QString Agent::toQString(const Agent::Type e)
{
    const QMetaObject &QMO = Agent::staticMetaObject;
    QMetaEnum QME = QMO.enumerator(QMO.indexOfEnumerator("Type"));
    return QString("%1").arg(QME.valueToKey(e));
}

//! Output Agent type enum to stream
inline QTextStream &operator<<(QTextStream &stream,const Agent::Type e)
{
    return stream << Agent::toQString(e);
}

//! Output Agent type enum to debug information
inline QDebug operator<<(QDebug dbg,const Agent::Type e)
{
    return dbg << qPrintable(Agent::toQString(e));
}

}
#endif
