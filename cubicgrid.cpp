#include "cubicgrid.h"
#include <cmath>
#include <QDebug>

namespace Langmuir
{

  using Eigen::Vector3d;

  CubicGrid::CubicGrid( unsigned int width, unsigned int height, unsigned int depth ) :
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

  QVector<unsigned int> CubicGrid::neighbors(unsigned int site, unsigned int hoppingRange)
  {

    // Return the indexes of all nearest neighbours
    QVector<unsigned int>      nList(0);
    unsigned int col = getColumn(site);
    unsigned int row =    getRow(site);
    unsigned int lay =  getLayer(site);

    switch ( hoppingRange )
    {
     case 1:
     {
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
      if (col == 0)
        nList.push_back(getIndex(m_width-1,m_height-1,m_depth-1) + 1);
      if (col == m_width - 1)
        nList.push_back(getIndex(m_width-1,m_height-1,m_depth-1) + 2);
      // Now we have all the nearest neighbours - between 4 and 6 in this case
      return nList;
      break;
     }
     case 2:
     {
       if (col > 0)
        {
         nList.push_back( getIndex(col-1,row,lay));
         if (row < m_height -1)
          nList.push_back( getIndex(col-1,row+1,lay));
         if (row > 0)
          nList.push_back( getIndex(col-1,row-1,lay));
         if (col > 1)
          nList.push_back( getIndex(col-2,row,lay));
        }
       if (col < m_width - 1)
        {
         nList.push_back( getIndex(col+1,row,lay));
         if (row < m_height - 1)
          nList.push_back( getIndex(col+1,row+1,lay));
         if (row > 0)
          nList.push_back( getIndex(col+1,row-1,lay));
         if (col < m_width - 2)
          nList.push_back( getIndex(col+2,row,lay));
        }
       if (row > 0)
        {
         nList.push_back( getIndex(col,row-1,lay));
         if (row > 1)
          nList.push_back( getIndex(col,row-2,lay));
        }
       if (row < m_height - 1)
        {
         nList.push_back( getIndex(col,row+1,lay));
         if (row < m_height - 2)
          nList.push_back( getIndex(col,row+2,lay));
        }
       if (lay > 0)
        {
         nList.push_back( getIndex(col,row,lay-1));
         if (lay > 1)
          nList.push_back( getIndex(col,row,lay-2));
         if (col > 0)
          {
           nList.push_back( getIndex(col-1,row,lay-1));
           if (row < m_height -1)
            nList.push_back( getIndex(col-1,row+1,lay-1));
           if (row > 0)
            nList.push_back( getIndex(col-1,row-1,lay-1));
          }
         if (col < m_width - 1)
          {
           nList.push_back( getIndex(col+1,row,lay-1));
           if (row < m_height - 1)
            nList.push_back( getIndex(col+1,row+1,lay-1));
           if (row > 0)
            nList.push_back( getIndex(col+1,row-1,lay-1));
          }
         if (row > 0)
          nList.push_back( getIndex(col,row-1,lay-1));
         if (row < m_height - 1)
          nList.push_back( getIndex(col,row+1,lay-1));
        }
       if (lay < m_depth-1)
        {
         nList.push_back( getIndex(col,row,lay+1));
         if (lay < m_depth-2)
          nList.push_back( getIndex(col,row,lay+2));
         if (col > 0)
          {
           nList.push_back( getIndex(col-1,row,lay+1));
           if (row < m_height -1)
            nList.push_back( getIndex(col-1,row+1,lay+1));
           if (row > 0)
            nList.push_back( getIndex(col-1,row-1,lay+1));
          }
         if (col < m_width - 1)
          {
           nList.push_back( getIndex(col+1,row,lay+1));
           if (row < m_height - 1)
            nList.push_back( getIndex(col+1,row+1,lay+1));
           if (row > 0)
            nList.push_back( getIndex(col+1,row-1,lay+1));
          }
         if (row > 0)
          nList.push_back( getIndex(col,row-1,lay+1));
         if (row < m_height - 1)
          nList.push_back( getIndex(col,row+1,lay+1));
        }
      if (col == 0)
        nList.push_back(getIndex(m_width-1,m_height-1,m_depth-1) + 1);
      if (col == m_width - 1)
        nList.push_back(getIndex(m_width-1,m_height-1,m_depth-1) + 2);
      return nList;
      break;
     }
     default:
     {
      int cutoff = hoppingRange * hoppingRange;
      for ( int z = (lay - hoppingRange); z < int(lay + hoppingRange + 1); z += 1 )
      {
       if ( z >= 0 && z < int(m_depth) )
       {
        for ( int x = (col - hoppingRange); x < int(col + hoppingRange + 1); x += 1 )
        {
         if ( x >= 0 && x < int(m_width) )
         {
          for ( int y = (row - hoppingRange); y < int(row + hoppingRange + 1); y += 1 )
          {
           if ( y >= 0 && y < int(m_height) )
           {
            if ( !(x == int(col) && y == int(row) && z == int(lay)) )
            {
             int r2 = ( col - x ) * ( col - x ) + ( row - y ) * ( row - y ) + ( lay - z ) * ( lay - z );
             if ( r2 <= cutoff )
             {
              nList.push_back(getIndex(x,y,z));
             }
            }
           }
          }
         }
        }
       }
      }
      if (col == 0)
        nList.push_back(getIndex(m_width-1,m_height-1,m_depth-1) + 1);
      if (col == m_width - 1)
        nList.push_back(getIndex(m_width-1,m_height-1,m_depth-1) + 2);
      return nList;
      break;
     }
    }

  }

  QVector<unsigned int> CubicGrid::row(unsigned int row, unsigned int layer)
  {
    QVector<unsigned int> vRow;
    for (unsigned int i = 0; i < m_width; i++) {
      vRow.push_back(getIndex(i, row, layer));
    }
    return vRow;
  }

  QVector<unsigned int> CubicGrid::col(unsigned int col, unsigned int layer)
  {
    QVector<unsigned int> vCol;
    for (unsigned int i = 0; i < m_height; i++) {
      vCol.push_back(getIndex(col, i, layer));
    }
    return vCol;
  }

  void CubicGrid::setSize(unsigned int width, unsigned int height, unsigned int depth)
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

  unsigned int CubicGrid::width()
  {
    return m_width;
  }

  unsigned int CubicGrid::height()
  {
    return m_height;
  }

  unsigned int CubicGrid::depth()
  {
    return m_depth;
  }

  unsigned int CubicGrid::area()
  {
    return m_area;
  }

  unsigned int CubicGrid::volume()
  {
    return m_volume;
  }

  double CubicGrid::totalDistance(unsigned int site1, unsigned int site2)
  {
    return sqrt(    (    getRow(site1) -    getRow(site2) ) * (    getRow(site1) -    getRow(site2) ) +
                    ( getColumn(site1) - getColumn(site2) ) * ( getColumn(site1) - getColumn(site2) ) +
                    (  getLayer(site1) -  getLayer(site2) ) * (  getLayer(site1) -  getLayer(site2) )
               );
  }

  double CubicGrid::xDistance(unsigned int site1, unsigned int site2)
  {
    return fabs(int(getColumn(site1)) - int(getColumn(site2)));
  }

  double CubicGrid::yDistance(unsigned int site1, unsigned int site2)
  {
    return fabs(int(getRow(site1)) - int(getRow(site2)));
  }

  double CubicGrid::zDistance(unsigned int site1, unsigned int site2)
  {
    return fabs(int(getLayer(site1)) - int(getLayer(site2)));
  }

  double CubicGrid::xImageDistance(unsigned int site1, unsigned int site2)
  {
    return fabs(int(getColumn(site1)) + int(getColumn(site2)))+1;
  }

  double CubicGrid::yImageDistance(unsigned int site1, unsigned int site2)
  {
    return fabs(int(getRow(site1)) + int(getRow(site2)))+1;
  }

  double CubicGrid::zImageDistance(unsigned int site1, unsigned int site2)
  {
    return fabs(int(getLayer(site1)) + int(getLayer(site2)))+1;
  }

  int CubicGrid::xDistancei(unsigned int site1, unsigned int site2)
  {
    return abs(int(getColumn(site1)) - int(getColumn(site2)));
  }

  int CubicGrid::yDistancei(unsigned int site1, unsigned int site2)
  {
    return abs(int(getRow(site1)) - int(getRow(site2)));
  }

  int CubicGrid::zDistancei(unsigned int site1, unsigned int site2)
  {
    return abs(int(getLayer(site1)) - int(getLayer(site2)));
  }

  int CubicGrid::xImageDistancei(unsigned int site1, unsigned int site2)
  {
    return abs(int(getColumn(site1)) + int(getColumn(site2)))+1;
  }

  int CubicGrid::yImageDistancei(unsigned int site1, unsigned int site2)
  {
    return abs(int(getRow(site1)) + int(getRow(site2)))+1;
  }

  int CubicGrid::zImageDistancei(unsigned int site1, unsigned int site2)
  {
    return abs(int(getLayer(site1)) + int(getLayer(site2)))+1;
  }

  unsigned int CubicGrid::getRow(unsigned int site)
  {
   return ( site / m_width - ( site / m_area ) * m_height );
  }

  unsigned int CubicGrid::getColumn(unsigned int site)
  {
    return site % m_width;
  }

  unsigned int CubicGrid::getLayer(unsigned int site)
  {
    return site / ( m_area );
  }

  unsigned int CubicGrid::getIndex(unsigned int column, unsigned int row, unsigned int layer )
  {
   return ( m_width * ( row + layer*m_height ) + column );
  }

  void CubicGrid::setAgent(unsigned int site, Agent *agent)
  {
   m_agents[site] = agent;
  }

  void CubicGrid::print3D( QTextStream& stream )
  {
   stream << m_agents.size()-2+2*m_height*m_depth << "\n\n";
   for ( unsigned int i = 0; i < m_agents.size()-2; i++ )
   {
    if ( m_agents[i] )
    {
     if ( siteID(i) == 0 )//A Charge
     {
      Eigen::Vector3d pos = position(i);
      stream << "A " << pos[0] << " " << pos[1] << " " << pos[2] << "\n";
     }
     else if ( siteID(i) == 1 )//A Defect or Trap
     {
      Eigen::Vector3d pos = position(i);
      stream << "B " << pos[0] << " " << pos[1] << " " << pos[2] << "\n";
     }
     else
     {
      qDebug() << "AGENT ERROR: " << siteID(i) << "\n";
      throw(-1);
     }
    }
    else
    {
     stream << "A -0.5 -0.5 -0.5" << "\n";
    }
  }

  for ( unsigned int i = 0; i < m_depth; i++ )
  {
   for ( unsigned int j = 0; j < m_height; j++ )
   {
    stream << "S " << -0.5 << " " << 0.5+j << " " << 0.5+i << "\n";
    stream << "D " << 0.5+m_width << " " << 0.5+j << " " << 0.5+i << "\n";
   }
  }
  stream.flush();
  }
} // End namespace Langmuir
