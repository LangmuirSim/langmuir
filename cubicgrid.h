#ifndef CUBICGRID_H
#define CUBICGRID_H

#include "agent.h"
#include <QtCore>

namespace Langmuir
{

class Grid
{
public:   
    enum CubeFace
    {
        Left       =   0,
        Right      =   1,
        Top        =   2,
        Bottom     =   3,
        Front      =   4,
        Back       =   5,
        NoFace     =   6,
        SIZE       =   7
    };
    static QString cubeFaceToQString(const Grid::CubeFace &cubeFace);
    Grid( int width = 0, int height = 0, int depth = 1 );
    ~Grid();
    int width();
    int height();
    int depth();
    int area();
    int volume();
    double totalDistance(int site1, int site2);
    double xDistance(int site1, int site2);
    double yDistance(int site1, int site2);
    double zDistance(int site1, int site2);
    double xImageDistance(int site1, int site2);
    double yImageDistance(int site1, int site2);
    double zImageDistance(int site1, int site2);
    int xDistancei(int site1, int site2);
    int yDistancei(int site1, int site2);
    int zDistancei(int site1, int site2);
    int xImageDistancei(int site1, int site2);
    int yImageDistancei(int site1, int site2);
    int zImageDistancei(int site1, int site2);
    int getIndex(int column, int row, int layer = 0);
    void setAgentAddress(int site, Agent *agentAddress);
    int getRow(int site);
    int getColumn(int site);
    int getLayer(int site);
    Agent * agentAddress(int site);
    void setAgentType(int site, Agent::Type id);
    Agent::Type agentType(int site);
    void addToPotential(int site, double potential);
    void setPotential(int site, double potential);
    double potential(int site);
    QVector<int> neighborsSite(int site);
    QVector<int> neighborsFace(Grid::CubeFace cubeFace);
    QVector<int> sliceIndex(int xi, int xf, int yi, int yf, int zi, int zf);

    void registerAgent(Agent *agent);
    void registerSpecialAgent(Agent *agent, Grid::CubeFace cubeFace);
    void unregisterAgent(Agent *agent);
    void unregisterSpecialAgent(Agent *agent, Grid::CubeFace cubeFace);
    int specialAgentCount();
    QList<Agent *>& getSpecialAgentList(Grid::CubeFace cubeFace);

protected:
    QVector<Agent *> m_agents;
    QVector<double> m_potentials;
    QVector<Agent::Type> m_agentType;
    QList< QList<Agent *> > m_specialAgents;
    int m_specialAgentReserve;
    int m_specialAgentCount;
    int m_width;
    int m_height;
    int m_depth;
    int m_area;
    int m_volume;
};
inline QDebug operator<<(QDebug dbg, const Grid::CubeFace &cubeFace)
{
    return dbg << qPrintable(Grid::cubeFaceToQString(cubeFace));
}
}
#endif
