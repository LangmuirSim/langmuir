#include "cubicgrid.h"
#include <cmath>

namespace Langmuir
{
Grid::Grid(int xSize, int ySize, int zSize, QObject *parent) : QObject(parent)
{
    m_xSize = xSize;
    m_ySize = ySize;
    m_zSize = zSize;
    m_xyPlaneArea = ySize*xSize;
    m_yzPlaneArea = ySize*zSize;
    m_xzPlaneArea = xSize*zSize;
    m_volume = ySize*xSize*zSize;
    m_specialAgentCount = 0;
    m_specialAgentReserve = 5;
    m_agents.fill(0, m_volume+m_specialAgentReserve*Grid::SIZE);
    m_potentials.fill(0.0, m_volume+m_specialAgentReserve*Grid::SIZE);
    m_agentType.fill(Agent::Empty, m_volume+m_specialAgentReserve*Grid::SIZE);
    m_specialAgents.reserve(Grid::SIZE);
    for(int i = 0; i < Grid::SIZE; i++)
    {
        QList<Agent*> qlist;
        m_specialAgents.push_back(qlist);
        m_specialAgents[i].reserve(m_specialAgentReserve);
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
        ndx_rev = 0;
    }
    if(yf < yi)
    {
        ndx_rev = yi;
        yi = yf;
        yf = ndx_rev;
        ndx_rev = 0;
    }
    if(zf < zi)
    {
        ndx_rev = zi;
        zi = zf;
        zf = ndx_rev;
        ndx_rev = 0;
    }
    if(xi <       0 || yi <        0 || zi <       0 ||
         xf <       0 || yf <        0 || zf <       0 ||
         xi > m_xSize || yi > m_ySize || zi > m_zSize ||
         xf > m_xSize || yf > m_ySize || zf > m_zSize)
    {
        qFatal("invalid slice index range:(%d, %d, %d)->(%d, %d, %d)", xi, yi, zi, xf, yf, zf);
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

QVector<int> Grid::neighborsSite(int site)
{
    // Return the indexes of all nearest neighbours
    QVector<int>     nList(0);
    int col = getIndexX(site);
    int row = getIndexY(site);
    int lay = getIndexZ(site);
    // To the west
    if(col > 0)
        nList.push_back(getIndexS(col-1, row, lay));
    // To the east
    if(col < m_xSize - 1)
        nList.push_back(getIndexS(col+1, row, lay));
    // To the south
    if(row > 0)
        nList.push_back(getIndexS(col, row-1, lay));
    // To the north
    if(row < m_ySize - 1)
        nList.push_back(getIndexS(col, row+1, lay));
    // Below
    if(lay > 0)
        nList.push_back(getIndexS(col, row, lay-1));
    // Above
    if(lay < m_zSize - 1)
        nList.push_back(getIndexS(col, row, lay+1));
    // Now for the drains....
    // Left
    if(col == 0)
    {
        foreach(Agent* agent, m_specialAgents[Grid::Left])
        {
            nList.push_back(agent->site());
        }
    }
    // Right
    if(col == m_xSize - 1)
    {
        foreach(Agent* agent, m_specialAgents[Grid::Right])
        {
            nList.push_back(agent->site());
        }
    }
    // Left
    if(row == 0)
    {
        foreach(Agent* agent, m_specialAgents[Grid::Top])
        {
            nList.push_back(agent->site());
        }
    }
    // Right
    if(row == m_ySize - 1)
    {
        foreach(Agent* agent, m_specialAgents[Grid::Bottom])
        {
            nList.push_back(agent->site());
        }
    }
    // Bottom
    if(lay == 0)
    {
        foreach(Agent* agent, m_specialAgents[Grid::Back])
        {
            nList.push_back(agent->site());
        }
    }
    // Top
    if(lay == m_zSize - 1)
    {
        foreach(Agent* agent, m_specialAgents[Grid::Front])
        {
            nList.push_back(agent->site());
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
        qFatal("can not generate neightbors for face; unknown face");
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
        qFatal("can not register special agent; exceeded the max special agents");
    }

    QList< Agent *>& specialAgents = getSpecialAgentList(cubeFace);
    if(specialAgents.contains(agent))
    {
        qFatal("can not register special agent; agent address already found");
    }
    specialAgents.push_back(agent);

    int site = m_volume+m_specialAgentCount;
    if(m_agents[site] == 0 && m_agentType[site] == Agent::Empty)
    {
        m_agents[site] = agent;
        m_agentType[site] = agent->type();
    }
    else
    {
        qFatal("can not register special agent: site is already occupied");
    }

    QVector<int> neighbors = neighborsFace(cubeFace);
    agent->setNeighbors(neighbors);
    agent->setSite(site);
    agent->setSite(site, true);
    ++m_specialAgentCount;
}

void Grid::unregisterSpecialAgent(Agent *agent, Grid::CubeFace cubeFace)
{
    QList< Agent *>& specialAgents = getSpecialAgentList(cubeFace);
    if(!(specialAgents.count(agent)== 1))
    {
        qFatal("can not unregister special agent; agent address not found");
    }
    specialAgents.removeOne(agent);

    int site = agent->site();
    if(!(m_agents[site] == agent))
    {
        qFatal("can not unregister special agent! pointers do not match");
    }

    m_agentType[site] = Agent::Empty;
    m_agents[site] = 0;
    --m_specialAgentCount;
}

void Grid::registerAgent(Agent *agent)
{
    int site = agent->site();
    if(m_agents[site] == 0 && m_agentType[site] == Agent::Empty)
    {
        m_agents[site] = agent;
        m_agentType[site] = agent->type();
    }
    else
    {
        qFatal("can not register agent: site is already occupied");
    }
    QVector<int> neighbors = neighborsSite(site);
    agent->setNeighbors(neighbors);
}

void Grid::unregisterAgent(Agent *agent)
{
    int site = agent->site();
    if(!(m_agents[site] == agent))
    {
        qFatal("can not unregister agent! pointers do not match");
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
        qFatal("can not register defect: site is already occupied");
    }
}

void Grid::unregisterDefect(int site)
{
    if(m_agentType[site] != Agent::Defect || m_agents[site] != 0)
    {
        qFatal("can not unregister defect! type does not match");
    }
    m_agentType[site] = Agent::Empty;
    m_agents[site] = 0;
}

int Grid::specialAgentCount()
{
    return m_specialAgentCount;
}

QString Grid::cubeFaceToQString(const Grid::CubeFace &cubeFace)
{
    switch(cubeFace)
    {
    case Grid::Left:
    {
        return QString("Left");
        break;
    }

    case Grid::Right:
    {
        return QString("Right");
        break;
    }

    case Grid::Top:
    {
        return QString("Top");
        break;
    }

    case Grid::Bottom:
    {
        return QString("Bottom");
        break;
    }

    case Grid::Front:
    {
        return QString("Front");
        break;
    }

    case Grid::Back:
    {
        return QString("Back");
        break;
    }

    case Grid::NoFace:
    {
        return QString("NoFace");
        break;
    }

    case Grid::SIZE:
    {
        return QString("SIZE");
        break;
    }

    default:
    {
        return QString("UnknownCubeFace(%1)").arg(cubeFace);
        break;
    }
    }
    return QString("UnknownCubeFace(?)");
}

}
