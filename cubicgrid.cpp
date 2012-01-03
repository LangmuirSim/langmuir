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
    m_agents.resize( width*height*depth+extraAgentCount(),0);
    m_potentials.resize( width*height*depth+extraAgentCount(),0.0);
    m_agentType.resize( width*height*depth+extraAgentCount(),Agent::Empty);
    //The 4 extra sites are for:
    // +0: Agent::Type::SourceL
    // +1: Agent::Type::DrainL
    // +2: Agent::Type::SourceR
    // +3: Agent::Type::DrainR
    // +4: Agent::Type::Light
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

  void CubicGrid::setAgentAddress(int site, Agent *agent)
  {
   m_agents[site] = agent;
  }

  QVector<int> CubicGrid::sliceIndex(int xi, int xf, int yi, int yf, int zi, int zf)
  {
      int ndx_rev = 0;
      if ( xf < xi )
      {
          ndx_rev = xi;
          xi = xf;
          xf = ndx_rev;
          ndx_rev = 0;
      }
      if ( yf < yi )
      {
          ndx_rev = yi;
          yi = yf;
          yf = ndx_rev;
          ndx_rev = 0;
      }
      if ( zf < zi )
      {
          ndx_rev = zi;
          zi = zf;
          zf = ndx_rev;
          ndx_rev = 0;
      }
      if ( xi <       0 || yi <        0 || zi <       0 ||
           xf <       0 || yf <        0 || zf <       0 ||
           xi > m_width || yi > m_height || zi > m_depth ||
           xf > m_width || yf > m_height || zf > m_depth    )
      {
          qFatal("invalid slice index range: (%d,%d,%d) -> (%d,%d,%d)",xi,yi,zi,xf,yf,zf);
      }
      QVector<int> ndx;
      for ( int i = xi; i < xf; i++ ) {
          for ( int j = yi; j < yf; j++ ) {
              for ( int k = zi; k < zf; k++ ) {
                  ndx.push_back(getIndex(i, j, k));
              }
          }
      }
      qSort(ndx);
      return ndx;
  }

  QVector<int> CubicGrid::neighborsSite(int site)
  {
      // Return the indexes of all nearest neighbours
      QVector<int>     nList(0);
      int col = getColumn(site);
      int row = getRow(site);
      int lay = getLayer(site);
      // To the west
      if (col > 0)
        nList.push_back(getIndex(col-1,row,lay));
      // To the east
      if (col < m_width - 1)
        nList.push_back(getIndex(col+1,row,lay));
      // To the south
      if (row > 0)
        nList.push_back(getIndex(col,row-1,lay));
      // To the north
      if (row < m_height - 1)
        nList.push_back(getIndex(col,row+1,lay));
      // Below
      if (lay > 0)
        nList.push_back(getIndex(col,row,lay-1));
      // Above
      if (lay < m_depth - 1)
        nList.push_back(getIndex(col,row,lay+1));
      // Now for the drains....
      //if (col == 0)
      //  nList.push_back(getIndexDrainL()); //left drain
      //if (col == m_width - 1)
      //  nList.push_back(getIndexDrainR()); //right right
      // Now we have all the nearest neighbours - between 4 and 6 in this case
      return nList;
  } 
} // End namespace Langmuir
