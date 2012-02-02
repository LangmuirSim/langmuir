#ifndef AGENT_H
#define AGENT_H

#include <QTextStream>
#include <QMetaObject>
#include <QMetaEnum>
#include <QVector>
#include <QObject>
#include <QString>
#include <QDebug>

namespace Langmuir
{

class World;

class Agent : public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(Agent)
    Q_ENUMS(Type)

public:
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

public:
    Agent(Type getType, World &world, int site = 0, QObject *parent = 0);
    virtual ~Agent();

    const QVector<int>& getNeighbors() const;
    void setNeighbors(QVector<int> neighbors);

    int getCurrentSite() const;
    int getFutureSite() const;

    void setCurrentSite(int site);
    void setFutureSite(int site);

    Type getType() const;
    World& getWorld() const;

protected:
    int m_site;
    int m_fSite;
    World &m_world;
    QVector<int> m_neighbors;
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
