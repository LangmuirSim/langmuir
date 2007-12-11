
#include "cubicgrid.h"

#include <cmath>

using namespace std;

namespace Langmuir{

  CubicGrid::CubicGrid()
  {
  }

  CubicGrid::CubicGrid(int width, int height) : m_width(width), m_height(height)
  {
  }

  CubicGrid::~CubicGrid()
  {
    
  }

  vector<int> CubicGrid::getNeighbors(int site)
  {
    // Return the indexes of all nearest neighbours
    vector<int> neighbors;
    int column = getColumn(site);
    int row = getRow(site);
    // To the left
    if (column > 0)
      neighbors.push_back(getIndex(column-1, row));
    // To the right
    if (column < m_width)
      neighbors.push_back(getIndex(column+1, row));
    // Up
    if (row > 0)
      neighbors.push_back(getIndex(column, row-1));
    // Down
    if (row < m_height)
      neighbors.push_back(getIndex(column, row+1));

    // Now we have all the nearest neighbours - between 2 and 4 in this case
    return neighbors;
  }

  void CubicGrid::setSize(int width, int height)
  {
    m_width = width;
    m_height = height;
  }

  int CubicGrid::getWidth()
  {
    return m_width;
  }

  int CubicGrid::getHeight()
  {
    return m_height;
  }

  double CubicGrid::getTotalDistance(int site1, int site2)
  {
    return sqrt((getRow(site1) - getRow(site2)) * (getRow(site1) - getRow(site2))
                + (getColumn(site1) - getColumn(site2)) * (getColumn(site1) - getColumn(site2)));
  }

  double CubicGrid::getXDistance(int site1, int site2)
  {
    return fabs(getRow(site1) - getRow(site2));
  }

  double CubicGrid::getYDistance(int site1, int site2)
  {
    return fabs(getColumn(site1) - getColumn(site2));
  }

  int CubicGrid::getRow(int site)
  {
    return site / m_width;
  }
  
  int CubicGrid::getColumn(int site)
  {
    return site % m_width;
  }

  int CubicGrid::getIndex(int column, int row)
  {
    return m_width * row + column;
  }

} // End namespace Langmuir
