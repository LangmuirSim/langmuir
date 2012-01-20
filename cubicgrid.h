#ifndef CUBICGRID_H
#define CUBICGRID_H

#include "agent.h"

#include <QTextStream>
#include <QVector>
#include <QString>
#include <QObject>
#include <QDebug>

namespace Langmuir
{

class Grid : public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(Grid)
    Q_ENUMS(CubeFace)

public:

    enum CubeFace
    {
        Left       =   0, // x =  0, yz plane
        Right      =   1, // x = lx, yz plane
        Top        =   2, // z =  0, xy plane
        Bottom     =   3, // z = lz, xy plane
        Front      =   4, // y =  0, xz plane
        Back       =   5, // y = ly, xz plane
        NoFace     =   6
    };
    static QString toQString(const Grid::CubeFace e);

    Grid(int xSize = 0, int ySize = 0, int zSize = 1, QObject *parent = 0);
    ~Grid();
    int xSize();
    int ySize();
    int zSize();
    int xyPlaneArea();
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
    int getIndexS(int xIndex, int yIndex, int zIndex = 0);
    int getIndexY(int site);
    int getIndexX(int site);
    int getIndexZ(int site);
    double getPositionY(int site);
    double getPositionX(int site);
    double getPositionZ(int site);
    Agent * agentAddress(int site);
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
    void unregisterDefect(int site);
    void registerDefect(int site);
    int specialAgentCount();
    QList<Agent *>& getSpecialAgentList(Grid::CubeFace cubeFace);

protected:
    QVector<Agent *> m_agents;
    QVector<double> m_potentials;
    QVector<Agent::Type> m_agentType;
    QList< QList<Agent *> > m_specialAgents;
    int m_specialAgentReserve;
    int m_specialAgentCount;
    int m_xSize;
    int m_ySize;
    int m_zSize;
    int m_xyPlaneArea;
    int m_yzPlaneArea;
    int m_xzPlaneArea;
    int m_volume;
};

QTextStream &operator<<(QTextStream &stream,const Grid::CubeFace e);
QDebug operator<<(QDebug dbg,const Grid::CubeFace e);

}
#endif
