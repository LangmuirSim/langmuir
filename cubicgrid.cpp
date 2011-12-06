#include "cubicgrid.h"
#include <cmath>

namespace Langmuir
{
  CubicGrid::CubicGrid( int width, int height, int depth ) :
      m_width  ( width              ), 
      m_height ( height             ), 
      m_depth  ( depth              ), 
      m_area   ( height*width       ), 
      m_volume ( height*width*depth )
  {
        m_agents.resize( width*height*depth+2,   0);
    m_potentials.resize( width*height*depth+2, 0.0);
        m_siteID.resize( width*height*depth+2,   0);
  }

  CubicGrid::~CubicGrid()
  {
  }

  QVector<int> CubicGrid::row(int row, int layer)
  {
    QVector<int> vRow;
    for (int i = 0; i < m_width; i++) {
      vRow.push_back(getIndex(i, row, layer));
    }
    return vRow;
  }

  QVector<int> CubicGrid::col(int col, int layer)
  {
    QVector<int> vCol;
    for (int i = 0; i < m_height; i++) {
      vCol.push_back(getIndex(col, i, layer));
    }
    return vCol;
  }

  void CubicGrid::setSize(int width, int height, int depth)
  {
    m_width  =  width;
    m_height = height;
    m_depth  =  depth;
    m_area   = width * depth;
    m_volume = width * depth * height;
    m_agents.resize( width*height*depth+2, 0);
    m_potentials.resize( width*height*depth+2, 0.0);
    m_siteID.resize( width*height*depth+2, 0);
  }

  int CubicGrid::width()
  {
    return m_width;
  }

  int CubicGrid::height()
  {
    return m_height;
  }

  int CubicGrid::depth()
  {
    return m_depth;
  }

  int CubicGrid::area()
  {
    return m_area;
  }

  int CubicGrid::volume()
  {
    return m_volume;
  }

  double CubicGrid::totalDistance(int site1, int site2)
  {
    return sqrt(    (    getRow(site1) -    getRow(site2) ) * (    getRow(site1) -    getRow(site2) ) +
                    ( getColumn(site1) - getColumn(site2) ) * ( getColumn(site1) - getColumn(site2) ) +
                    (  getLayer(site1) -  getLayer(site2) ) * (  getLayer(site1) -  getLayer(site2) )
               );
  }

  double CubicGrid::xDistance(int site1, int site2)
  {
    return fabs(int(getColumn(site1)) - int(getColumn(site2)));
  }

  double CubicGrid::yDistance(int site1, int site2)
  {
    return fabs(int(getRow(site1)) - int(getRow(site2)));
  }

  double CubicGrid::zDistance(int site1, int site2)
  {
    return fabs(int(getLayer(site1)) - int(getLayer(site2)));
  }

  double CubicGrid::xImageDistance(int site1, int site2)
  {
    return fabs(int(getColumn(site1)) + int(getColumn(site2)))+1;
  }

  double CubicGrid::yImageDistance(int site1, int site2)
  {
    return fabs(int(getRow(site1)) + int(getRow(site2)))+1;
  }

  double CubicGrid::zImageDistance(int site1, int site2)
  {
    return fabs(int(getLayer(site1)) + int(getLayer(site2)))+1;
  }

  int CubicGrid::xDistancei(int site1, int site2)
  {
    return abs(int(getColumn(site1)) - int(getColumn(site2)));
  }

  int CubicGrid::yDistancei(int site1, int site2)
  {
    return abs(int(getRow(site1)) - int(getRow(site2)));
  }

  int CubicGrid::zDistancei(int site1, int site2)
  {
    return abs(int(getLayer(site1)) - int(getLayer(site2)));
  }

  int CubicGrid::xImageDistancei(int site1, int site2)
  {
    return abs(int(getColumn(site1)) + int(getColumn(site2)))+1;
  }

  int CubicGrid::yImageDistancei(int site1, int site2)
  {
    return abs(int(getRow(site1)) + int(getRow(site2)))+1;
  }

  int CubicGrid::zImageDistancei(int site1, int site2)
  {
    return abs(int(getLayer(site1)) + int(getLayer(site2)))+1;
  }

  int CubicGrid::getRow(int site)
  {
   return ( site / m_width - ( site / m_area ) * m_height );
  }

  int CubicGrid::getColumn(int site)
  {
    return site % m_width;
  }

  int CubicGrid::getLayer(int site)
  {
    return site / ( m_area );
  }

  int CubicGrid::getIndex(int column, int row, int layer )
  {
   return ( m_width * ( row + layer*m_height ) + column );
  }

  void CubicGrid::setAgent(int site, Agent *agent)
  {
   m_agents[site] = agent;
  }
} // End namespace Langmuir
