#include "cubicgrid.h"
#include <cmath>
#include <cstdlib>
#include <QDebug>

using namespace std;

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

  vector<unsigned int> CubicGrid::neighbors(unsigned int site)
  {
    // Return the indexes of all nearest neighbours
    vector<unsigned int>     neighbors(0);
    unsigned int column = getColumn(site);
    unsigned int row    =    getRow(site);
    unsigned int layer  =  getLayer(site);

    // To the west
    if (column > 0)
      neighbors.push_back(getIndex(column-1, row, layer));
    // To the east
    if (column < m_width - 1)
      neighbors.push_back(getIndex(column+1, row, layer));

    // To the south
    if (row > 0)
      neighbors.push_back(getIndex(column, row-1, layer));
    // To the north
    if (row < m_height - 1)
      neighbors.push_back(getIndex(column, row+1, layer));
 
    // Below
    if (layer > 0)
      neighbors.push_back(getIndex(column, row, layer-1));
    // Above
    if (layer < m_depth - 1)
      neighbors.push_back(getIndex(column, row, layer+1));

    // Now for the source and the drain....
    if (column == 0)
      neighbors.push_back(getIndex(m_width-1, m_height-1,m_depth-1) + 1);
    if (column == m_width - 1)
      neighbors.push_back(getIndex(m_width-1, m_height-1,m_depth-1) + 2);

    // Now we have all the nearest neighbours - between 4 and 6 in this case
    return neighbors;
  }

  vector<unsigned int> CubicGrid::row(unsigned int row, unsigned int layer)
  {
    vector<unsigned int> vRow;
    for (unsigned int i = 0; i < m_width; i++) {
      vRow.push_back(getIndex(i, row, layer));
    }
    return vRow;
  }

  vector<unsigned int> CubicGrid::col(unsigned int col, unsigned int layer)
  {
    vector<unsigned int> vCol;
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

  std::vector<Agent *> CubicGrid::neighborAgents(unsigned int site)
  {
    // Return the indexes of all nearest neighbours
    vector<Agent *>             neighbors;
    unsigned int column = getColumn(site);
    unsigned int row    =    getRow(site);
    unsigned int layer  =  getLayer(site);

    // To the west
    if (column > 0)
      neighbors.push_back(agent(getIndex(column-1, row, layer)));
    // To the east
    if (column < m_width - 1)
      neighbors.push_back(agent(getIndex(column+1, row, layer)));

    // To the south
    if (row > 0)
      neighbors.push_back(agent(getIndex(column, row-1, layer)));
    // To the north 
    if (row < m_height - 1)
      neighbors.push_back(agent(getIndex(column, row+1, layer)));

    // Below
    if (layer > 0)
      neighbors.push_back(agent(getIndex(column, row, layer-1)));
    // Above
    if (layer < m_depth - 1)
      neighbors.push_back(agent(getIndex(column, row, layer+1)));

    // Add the source agent
    if (column == 0)
      neighbors.push_back(agent(getIndex(m_width-1, m_height-1, m_depth-1) + 1));

    // Add the drain agent
    if (column == m_width - 1)
      neighbors.push_back(agent(getIndex(m_width-1, m_height-1, m_depth-1) + 2));

    // Now we have all the nearest neighbours - between 4 and 6 in this case
    return neighbors;

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
      cout << "AGENT ERROR: " << siteID(i) << "\n";
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
