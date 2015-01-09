#include "cubicgrid.h"
#include <cmath>
#include "world.h"
#include "parameters.h"
#include "drainagent.h"

namespace LangmuirCore
{
Grid::Grid(World &world, QObject *parent)
    : QObject(parent), m_world(world)
{
    m_xSize = m_world.parameters().gridX;
    m_ySize = m_world.parameters().gridY;
    m_zSize = m_world.parameters().gridZ;
    m_xyPlaneArea = m_world.parameters().gridY * m_world.parameters().gridX;
    m_yzPlaneArea = m_world.parameters().gridY * m_world.parameters().gridZ;
    m_xzPlaneArea = m_world.parameters().gridX * m_world.parameters().gridZ;
    m_volume = m_world.parameters().gridY *
               m_world.parameters().gridX *
               m_world.parameters().gridZ;
    m_specialAgentCount = 0;
    m_specialAgentReserve = 5*7;
    m_agents.fill(0, m_volume+m_specialAgentReserve);
    m_potentials.fill(0.0, m_volume+m_specialAgentReserve);
    m_agentType.fill(Agent::Empty, m_volume+m_specialAgentReserve);
    m_specialAgents.reserve(m_specialAgentReserve);
    for(int i = 0; i < 7; i++)
    {
        QList<Agent*> qlist;
        m_specialAgents.push_back(qlist);
        m_specialAgents[i].reserve(5);
    }
}

Grid::~Grid()
{
}

int Grid::xSize()
{
    return m_xSize;
}

int Grid::ySize()
{
    return m_ySize;
}

int Grid::zSize()
{
    return m_zSize;
}

int Grid::xyPlaneArea()
{
    return m_xyPlaneArea;
}

int Grid::volume()
{
    return m_volume;
}

double Grid::totalDistance(int site1, int site2)
{
    return sqrt((    getIndexY(site1)-    getIndexY(site2)) *(getIndexY(site1)-    getIndexY(site2)) +
(getIndexX(site1)- getIndexX(site2)) *(getIndexX(site1)- getIndexX(site2)) +
(getIndexZ(site1)-  getIndexZ(site2)) *(getIndexZ(site1)-  getIndexZ(site2))
);
}

double Grid::xDistance(int site1, int site2)
{
    return fabs(int(getIndexX(site1)) - int(getIndexX(site2)));
}

double Grid::yDistance(int site1, int site2)
{
    return fabs(int(getIndexY(site1)) - int(getIndexY(site2)));
}

double Grid::zDistance(int site1, int site2)
{
    return fabs(int(getIndexZ(site1)) - int(getIndexZ(site2)));
}

double Grid::xImageDistance(int site1, int site2)
{
    return fabs(int(getIndexX(site1)) + int(getIndexX(site2)))+1;
}

double Grid::yImageDistance(int site1, int site2)
{
    return fabs(int(getIndexY(site1)) + int(getIndexY(site2)))+1;
}

double Grid::zImageDistance(int site1, int site2)
{
    return fabs(int(getIndexZ(site1)) + int(getIndexZ(site2)))+1;
}

int Grid::xDistancei(int site1, int site2)
{
    return abs(int(getIndexX(site1)) - int(getIndexX(site2)));
}

int Grid::yDistancei(int site1, int site2)
{
    return abs(int(getIndexY(site1)) - int(getIndexY(site2)));
}

int Grid::zDistancei(int site1, int site2)
{
    return abs(int(getIndexZ(site1)) - int(getIndexZ(site2)));
}

int Grid::xImageDistancei(int site1, int site2)
{
    return abs(int(getIndexX(site1)) + int(getIndexX(site2)))+1;
}

int Grid::yImageDistancei(int site1, int site2)
{
    return abs(int(getIndexY(site1)) + int(getIndexY(site2)))+1;
}

int Grid::zImageDistancei(int site1, int site2)
{
    return abs(int(getIndexZ(site1)) + int(getIndexZ(site2)))+1;
}

int Grid::getIndexX(int site)
{
    return site % m_xSize;
}

int Grid::getIndexY(int site)
{
    return(site / m_xSize -(site / m_xyPlaneArea)* m_ySize);
}

int Grid::getIndexZ(int site)
{
    return site /(m_xyPlaneArea);
}

double Grid::getPositionX(int site)
{
    return getIndexX(site)+ 0.5;
}

double Grid::getPositionY(int site)
{
    return getIndexY(site)+ 0.5;
}

double Grid::getPositionZ(int site)
{
    return getIndexZ(site)+ 0.5;
}

int Grid::getIndexS(int xIndex, int yIndex, int zIndex)
{
    return(m_xSize *(yIndex + zIndex*m_ySize)+ xIndex);
}

QVector<int> Grid::sliceIndex(int xi, int xf, int yi, int yf, int zi, int zf)
{
    int ndx_rev = 0;
    if(xf < xi)
    {
        ndx_rev = xi;
        xi = xf;
        xf = ndx_rev;
    }
    if(yf < yi)
    {
        ndx_rev = yi;
        yi = yf;
        yf = ndx_rev;
    }
    if(zf < zi)
    {
        ndx_rev = zi;
        zi = zf;
        zf = ndx_rev;
    }
    if(  xi <       0 || yi <        0 || zi <       0 ||
         xf <       0 || yf <        0 || zf <       0 ||
         xi > m_xSize || yi > m_ySize || zi > m_zSize  ||
         xf > m_xSize || yf > m_ySize || zf > m_zSize)
    {
        qFatal("langmuir: invalid slice index range:(%d, %d, %d)->(%d, %d, %d)", xi, yi, zi, xf, yf, zf);
    }
    QVector<int> ndx;
    for(int i = xi; i < xf; i++){
        for(int j = yi; j < yf; j++){
            for(int k = zi; k < zf; k++){
                ndx.push_back(getIndexS(i, j, k));
            }
        }
    }
    qSort(ndx);
    return ndx;
}

QVector<int> Grid::neighborsSite(int site, int hoppingRange)
{
    // Return the indexes of all nearest neighbours
    QVector<int>   nList(0);
    int x = getIndexX(site);
    int y = getIndexY(site);
    int z = getIndexZ(site);

//    qDebug() << qPrintable(QString("GRID: xsize(%1) ysize(%2) zsize(%3)").arg(m_xSize,3).arg(m_ySize,3).arg(m_zSize,3));
//    qDebug() << qPrintable(QString("SITE:     s(%1)     x(%2)     y(%3)     z(%4)     t(%5)")
//                           .arg(site,3)
//                           .arg(x,3)
//                           .arg(y,3)
//                           .arg(z,3)
//                           .arg(Agent::toQString(m_agentType[site])));
    switch (hoppingRange)
    {
        case 1:
        {
            // To the west
            if(x > 0)
                nList.push_back(getIndexS(x-1, y, z));
            // To the east
            if(x < m_xSize - 1)
                nList.push_back(getIndexS(x+1, y, z));
            // To the south
            if(y > 0)
                nList.push_back(getIndexS(x, y-1, z));
            // To the north
            if(y < m_ySize - 1)
                nList.push_back(getIndexS(x, y+1, z));
            // Below
            if(z > 0)
                nList.push_back(getIndexS(x, y, z-1));
            // Above
            if(z < m_zSize - 1)
                nList.push_back(getIndexS(x, y, z+1));
            break;
        }
        case 2:
        {
            // Optimized for 1 layer
            if (m_zSize == 1)
            {
                //qDebug() << "special case: m_zSize == 1";
                // To the west 1
                if (x > 0)
                {
                    nList.push_back(getIndexS(x-1, y, z));
                    // To the south 1
                    if (y > 0)
                    {
                        nList.push_back(getIndexS(x-1, y-1, z));
                    }
                    // To the north 1
                    if (y < m_ySize - 1)
                    {
                        nList.push_back(getIndexS(x-1, y+1, z));
                    }
                    // To the west 2
                    if (x > 1)
                        nList.push_back(getIndexS(x-2, y, z));
                }

                // To the east 1
                if (x < m_xSize - 1)
                {
                    nList.push_back(getIndexS(x+1, y, z));
                    // To the south 1
                    if (y > 0)
                    {
                        nList.push_back(getIndexS(x+1, y-1, z));
                    }
                    // To the north 1
                    if (y < m_ySize - 1)
                    {
                        nList.push_back(getIndexS(x+1, y+1, z));
                    }
                    // To the east 2
                    if (x < m_xSize - 2)
                        nList.push_back(getIndexS(x+2, y, z));
                }

                // To the south 1
                if (y > 0)
                {
                    nList.push_back(getIndexS(x, y-1, z));
                    // To the south 2
                    if (y > 1)
                        nList.push_back(getIndexS(x, y-2, z));
                }

                // To the north 2
                if (y < m_ySize - 1)
                {
                    nList.push_back(getIndexS(x, y+1, z));
                    // To the north 2
                    if (y < m_ySize - 2)
                        nList.push_back(getIndexS(x, y+2, z));
                }
            }
            else
            {
                // To the west 1
                if (x > 0)
                {
                    nList.push_back(getIndexS(x-1, y, z));
                    // To the south 1
                    if (y > 0)
                    {
                        nList.push_back(getIndexS(x-1, y-1, z));
                        // To the down 1
                        if (z > 0)
                            nList.push_back(getIndexS(x-1, y-1, z-1));
                        // To the up 1
                        if (z < m_zSize - 1)
                            nList.push_back(getIndexS(x-1, y-1, z+1));
                    }
                    // To the north 1
                    if (y < m_ySize - 1)
                    {
                        nList.push_back(getIndexS(x-1, y+1, z));
                        // To the down 1
                        if (z > 0)
                            nList.push_back(getIndexS(x-1, y+1, z-1));
                        // To the up 1
                        if (z < m_zSize - 1)
                            nList.push_back(getIndexS(x-1, y+1, z+1));
                    }
                    if (z > 0)
                        nList.push_back(getIndexS(x-1, y, z-1));
                    // To the up 1
                    if (z < m_zSize - 1)
                        nList.push_back(getIndexS(x-1, y, z+1));
                    // To the west 2
                    if (x > 1)
                        nList.push_back(getIndexS(x-2, y, z));
                }

                // To the east 1
                if (x < m_xSize - 1)
                {
                    nList.push_back(getIndexS(x+1, y, z));
                    // To the south 1
                    if (y > 0)
                    {
                        nList.push_back(getIndexS(x+1, y-1, z));
                        // To the down 1
                        if (z > 0)
                            nList.push_back(getIndexS(x+1, y-1, z-1));
                        // To the up 1
                        if (z < m_zSize - 1)
                            nList.push_back(getIndexS(x+1, y-1, z+1));
                    }
                    // To the north 1
                    if (y < m_ySize - 1)
                    {
                        nList.push_back(getIndexS(x+1, y+1, z));
                        // To the down 1
                        if (z > 0)
                            nList.push_back(getIndexS(x+1, y+1, z-1));
                        // To the up 1
                        if (z < m_zSize - 1)
                            nList.push_back(getIndexS(x+1, y+1, z+1));
                    }
                    if (z > 0)
                        nList.push_back(getIndexS(x+1, y, z-1));
                    // To the up 1
                    if (z < m_zSize - 1)
                        nList.push_back(getIndexS(x+1, y, z+1));
                    // To the east 2
                    if (x < m_xSize - 2)
                        nList.push_back(getIndexS(x+2, y, z));
                }

                // To the down 1
                if (z > 0)
                {
                    nList.push_back(getIndexS(x, y, z-1));
                    // To the south 1
                    if (y > 0)
                    {
                        nList.push_back(getIndexS(x, y-1, z-1));
                    }
                    // To the north 1
                    if (y < m_ySize - 1)
                    {
                        nList.push_back(getIndexS(x, y+1, z-1));
                    }
                    // To the down 2
                    if (z > 1)
                        nList.push_back(getIndexS(x, y, z-2));
                }

                // To the up 1
                if (z < m_zSize - 1)
                {
                    nList.push_back(getIndexS(x, y, z+1));
                    // To the south 1
                    if (y > 0)
                    {
                        nList.push_back(getIndexS(x, y-1, z+1));
                    }
                    // To the north 1
                    if (y < m_ySize - 1)
                    {
                        nList.push_back(getIndexS(x, y+1, z+1));
                    }
                    // To the up 2
                    if (z < m_zSize - 2)
                        nList.push_back(getIndexS(x, y, z+2));
                }

                // To the south 1
                if (y > 0)
                {
                    nList.push_back(getIndexS(x, y-1, z));
                    // To the south 2
                    if (y > 1)
                        nList.push_back(getIndexS(x, y-2, z));
                }

                // To the north 2
                if (y < m_ySize - 1)
                {
                    nList.push_back(getIndexS(x, y+1, z));
                    // To the north 2
                    if (y < m_ySize - 2)
                        nList.push_back(getIndexS(x, y+2, z));
                }
            }
            break;
        }
        default:
        {
            qFatal("langmuir: invalid neighbor list size parameter : (%d)", hoppingRange);
            break;
        }
    }

    // Now for the drains....
    if (x == 0)
    {
        foreach (Agent *agent, getSpecialAgentList(Left))
        {
            if (agent->getType() == Agent::Drain)
            {
                nList.push_back(agent->getCurrentSite());
            }
        }
    }

    if(x == m_xSize - 1)
    {
        foreach (Agent *agent, getSpecialAgentList(Right))
        {
            if (agent->getType() == Agent::Drain)
            {
                nList.push_back(agent->getCurrentSite());
            }
        }
    }

    return nList;
}

QVector<int> Grid::neighborsFace(Grid::CubeFace cubeFace)
{
    switch(cubeFace)
    {
    case Grid::Left:
    {
        return sliceIndex(0, 1, 0, m_ySize, 0, m_zSize);
        break;
    }

    case Grid::Right:
    {
        return sliceIndex(m_xSize-1, m_xSize, 0, m_ySize, 0, m_zSize);
        break;
    }

    case Grid::Top:
    {
        return sliceIndex(0, m_xSize, 0, 1, 0, m_zSize);
        break;
    }

    case Grid::Bottom:
    {
        return sliceIndex(0, m_xSize, m_ySize-1, m_ySize, 0, m_zSize);
        break;
    }

    case Grid::Front:
    {
        return sliceIndex(0, m_xSize, 0, m_ySize, m_zSize-1, m_zSize);
        break;
    }

    case Grid::Back:
    {
        return sliceIndex(0, m_xSize, 0, m_ySize, 0, 1);
        break;
    }

    case Grid::NoFace:
    {
        return QVector<int>();
        break;
    }

    default:
    {
        qFatal("langmuir: can not generate neightbors for face; unknown face");
        return QVector<int>();
        break;
    }
    }
}

Agent * Grid::agentAddress(int site)
{
    return m_agents[site];
}

Agent::Type Grid::agentType(int site)
{
    return m_agentType[site];
}

void Grid::setPotential(int site, double potential)
{
    m_potentials[site] = potential;
}

void Grid::addToPotential(int site, double potential)
{
    m_potentials[site] = m_potentials[site] + potential;
}

double Grid::potential(int site)
{
    return m_potentials[site];
}

QList<Agent *>& Grid::getSpecialAgentList(Grid::CubeFace cubeFace)
{
    return m_specialAgents[cubeFace];
}

void Grid::registerSpecialAgent(Agent *agent, Grid::CubeFace cubeFace)
{
    if(m_specialAgentCount >= m_specialAgentReserve)
    {
        qFatal("langmuir: can not register special agent; exceeded the max special agents; %d >= %d",
               m_specialAgentCount,m_specialAgentReserve);
    }

    QList< Agent *>& specialAgents = getSpecialAgentList(cubeFace);
    if(specialAgents.contains(agent))
    {
        qFatal("langmuir: can not register special agent; agent address already found");
    }
    specialAgents.push_back(agent);

    int site = m_volume+m_specialAgentCount;
    if(m_agents[site] == 0 && m_agentType[site] == Agent::Empty)
    {
        m_agents[site] = agent;
        m_agentType[site] = agent->getType();
    }
    else
    {
        qFatal("langmuir: can not register special agent: site is already occupied");
    }

    QVector<int> neighbors = neighborsFace(cubeFace);
    agent->setNeighbors(neighbors);
    agent->setCurrentSite(site);
    agent->setFutureSite(site);
    ++m_specialAgentCount;
}

void Grid::unregisterSpecialAgent(Agent *agent, Grid::CubeFace cubeFace)
{
    QList< Agent *>& specialAgents = getSpecialAgentList(cubeFace);
    if(!(specialAgents.count(agent)== 1))
    {
        qFatal("langmuir: can not unregister special agent; agent address not found");
    }
    specialAgents.removeOne(agent);

    int site = agent->getCurrentSite();
    if(!(m_agents[site] == agent))
    {
        qFatal("langmuir: can not unregister special agent! pointers do not match");
    }

    m_agentType[site] = Agent::Empty;
    m_agents[site] = 0;
    --m_specialAgentCount;
}

void Grid::registerAgent(Agent *agent)
{
    int site = agent->getCurrentSite();
    if((m_agents[site] == 0) && (m_agentType[site] == Agent::Empty))
    {
        m_agents[site] = agent;
        m_agentType[site] = agent->getType();
    }
    else
    {
        qFatal("langmuir: can not register agent: site %d is invalid", site);
    }
    QVector<int> neighbors = neighborsSite(site, m_world.parameters().hoppingRange);
    agent->setNeighbors(neighbors);
}

void Grid::unregisterAgent(Agent *agent)
{
    int site = agent->getCurrentSite();
    if(!(m_agents[site] == agent))
    {
        qFatal("langmuir: can not unregister agent! pointers do not match");
    }
    m_agentType[site] = Agent::Empty;
    m_agents[site] = 0;
}

void Grid::registerDefect(int site)
{
    if(m_agents[site] == 0 && m_agentType[site] == Agent::Empty)
    {
        m_agents[site] = 0;
        m_agentType[site] = Agent::Defect;
    }
    else
    {
        qFatal("langmuir: can not register defect: site is already occupied");
    }
}

void Grid::unregisterDefect(int site)
{
    if(m_agentType[site] != Agent::Defect || m_agents[site] != 0)
    {
        qFatal("langmuir: can not unregister defect! type does not match");
    }
    m_agentType[site] = Agent::Empty;
    m_agents[site] = 0;
}

int Grid::specialAgentCount()
{
    return m_specialAgentCount;
}

QString Grid::toQString(const Grid::CubeFace e)
{
    const QMetaObject &QMO = Grid::staticMetaObject;
    QMetaEnum QME = QMO.enumerator(QMO.indexOfEnumerator("CubeFace"));
    return QString("%1").arg(QME.valueToKey(e));
}

QTextStream &operator<<(QTextStream &stream,const Grid::CubeFace e)
{
    return stream << Grid::toQString(e);
}

QDebug operator<<(QDebug dbg,const Grid::CubeFace e)
{
    return dbg << qPrintable(Grid::toQString(e));
}

}
