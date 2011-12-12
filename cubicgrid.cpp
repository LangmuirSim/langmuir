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

  QVector<int> CubicGrid::slice(int xi, int xf, int yi, int yf, int zi, int zf)
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
      // Now for the source and the drain....
      //if (col == 0)
      //  nList.push_back(getIndex(m_width-1,m_height-1,m_depth-1) + 1);
      //if (col == m_width - 1)
      //  nList.push_back(getIndex(m_width-1,m_height-1,m_depth-1) + 2);
      // Now we have all the nearest neighbours - between 4 and 6 in this case
      return nList;
  } 
  /*
           case 2:
           {
             if (col > 0)
              {
               nList.push_back( m_world->grid()->getIndex(col-1,row,lay));
               if (row < m_world->grid()->height() -1)
                nList.push_back( m_world->grid()->getIndex(col-1,row+1,lay));
               if (row > 0)
                nList.push_back( m_world->grid()->getIndex(col-1,row-1,lay));
               if (col > 1)
                nList.push_back( m_world->grid()->getIndex(col-2,row,lay));
              }
             if (col < m_world->grid()->width() - 1)
              {
               nList.push_back( m_world->grid()->getIndex(col+1,row,lay));
               if (row < m_world->grid()->height() - 1)
                nList.push_back( m_world->grid()->getIndex(col+1,row+1,lay));
               if (row > 0)
                nList.push_back( m_world->grid()->getIndex(col+1,row-1,lay));
               if (col < m_world->grid()->width() - 2)
                nList.push_back( m_world->grid()->getIndex(col+2,row,lay));
              }
             if (row > 0)
              {
               nList.push_back( m_world->grid()->getIndex(col,row-1,lay));
               if (row > 1)
                nList.push_back( m_world->grid()->getIndex(col,row-2,lay));
              }
             if (row < m_world->grid()->height() - 1)
              {
               nList.push_back( m_world->grid()->getIndex(col,row+1,lay));
               if (row < m_world->grid()->height() - 2)
                nList.push_back( m_world->grid()->getIndex(col,row+2,lay));
              }
             if (lay > 0)
              {
               nList.push_back( m_world->grid()->getIndex(col,row,lay-1));
               if (lay > 1)
                nList.push_back( m_world->grid()->getIndex(col,row,lay-2));
               if (col > 0)
                {
                 nList.push_back( m_world->grid()->getIndex(col-1,row,lay-1));
                 if (row < m_world->grid()->height() -1)
                  nList.push_back( m_world->grid()->getIndex(col-1,row+1,lay-1));
                 if (row > 0)
                  nList.push_back( m_world->grid()->getIndex(col-1,row-1,lay-1));
                }
               if (col < m_world->grid()->width() - 1)
                {
                 nList.push_back( m_world->grid()->getIndex(col+1,row,lay-1));
                 if (row < m_world->grid()->height() - 1)
                  nList.push_back( m_world->grid()->getIndex(col+1,row+1,lay-1));
                 if (row > 0)
                  nList.push_back( m_world->grid()->getIndex(col+1,row-1,lay-1));
                }
               if (row > 0)
                nList.push_back( m_world->grid()->getIndex(col,row-1,lay-1));
               if (row < m_world->grid()->height() - 1)
                nList.push_back( m_world->grid()->getIndex(col,row+1,lay-1));
              }
             if (lay < m_world->grid()->depth()-1)
              {
               nList.push_back( m_world->grid()->getIndex(col,row,lay+1));
               if (lay < m_world->grid()->depth()-2)
                nList.push_back( m_world->grid()->getIndex(col,row,lay+2));
               if (col > 0)
                {
                 nList.push_back( m_world->grid()->getIndex(col-1,row,lay+1));
                 if (row < m_world->grid()->height() -1)
                  nList.push_back( m_world->grid()->getIndex(col-1,row+1,lay+1));
                 if (row > 0)
                  nList.push_back( m_world->grid()->getIndex(col-1,row-1,lay+1));
                }
               if (col < m_world->grid()->width() - 1)
                {
                 nList.push_back( m_world->grid()->getIndex(col+1,row,lay+1));
                 if (row < m_world->grid()->height() - 1)
                  nList.push_back( m_world->grid()->getIndex(col+1,row+1,lay+1));
                 if (row > 0)
                  nList.push_back( m_world->grid()->getIndex(col+1,row-1,lay+1));
                }
               if (row > 0)
                nList.push_back( m_world->grid()->getIndex(col,row-1,lay+1));
               if (row < m_world->grid()->height() - 1)
                nList.push_back( m_world->grid()->getIndex(col,row+1,lay+1));
              }
            if (col == 0)
              nList.push_back(m_world->grid()->getIndex(m_world->grid()->width()-1,m_world->grid()->height()-1,m_world->grid()->depth()-1) + 1);
            if (col == m_world->grid()->width() - 1)
              nList.push_back(m_world->grid()->getIndex(m_world->grid()->width()-1,m_world->grid()->height()-1,m_world->grid()->depth()-1) + 2);
            return nList;
            break;
           }
           default:
           {
            int cutoff = hoppingRange * hoppingRange;
            for ( int z = (lay - hoppingRange); z < int(lay + hoppingRange + 1); z += 1 )
            {
             if ( z >= 0 && z < int(m_world->grid()->depth()) )
             {
              for ( int x = (col - hoppingRange); x < int(col + hoppingRange + 1); x += 1 )
              {
               if ( x >= 0 && x < int(m_world->grid()->width()) )
               {
                for ( int y = (row - hoppingRange); y < int(row + hoppingRange + 1); y += 1 )
                {
                 if ( y >= 0 && y < int(m_world->grid()->height()) )
                 {
                  if ( !(x == int(col) && y == int(row) && z == int(lay)) )
                  {
                   int r2 = ( col - x ) * ( col - x ) + ( row - y ) * ( row - y ) + ( lay - z ) * ( lay - z );
                   if ( r2 <= cutoff )
                   {
                    nList.push_back(m_world->grid()->getIndex(x,y,z));
                   }
                  }
                 }
                }
               }
              }
             }
            }
            if (col == 0)
              nList.push_back(m_world->grid()->getIndex(m_world->grid()->width()-1,m_world->grid()->height()-1,m_world->grid()->depth()-1) + 1);
            if (col == m_world->grid()->width() - 1)
              nList.push_back(m_world->grid()->getIndex(m_world->grid()->width()-1,m_world->grid()->height()-1,m_world->grid()->depth()-1) + 2);
            return nList;
            break;
           }
          }
      }
  */

} // End namespace Langmuir
