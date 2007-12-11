/*
 *  CubicGrid - pure virtual base class to inherited by all grid classes
 *
 */

#ifndef __CUBICGRID_H
#define __CUBICGRID_H

#include "grid.h"

#include <vector>

namespace Langmuir{

  class CubicGrid {

  public:

    CubicGrid();

    CubicGrid(int width, int height);

    ~CubicGrid();
  
    /**
     * Get the nearest neighbours for the specified site.
     */
    std::vector<int> getNeighbors(int site);
  
    /**
     * Sets the size of the grid.
     */
    void setSize(int width, int height);
  
    /**
     * Gets the width of the grid.
     */
    int getWidth();
  
    /**
     * Gets the height fo the grid.
     */
    int getHeight();
  
    /**
     * Get the total distance between the two sites.
     */
    double getTotalDistance(int site1, int site2);
  
    /**
     * Get the x component of the distance between the two sites.
     */
    double getXDistance(int site1, int site2);
  
    /**
     * Get the x component of the distance between the two sites.
     */
    double getYDistance(int site1, int site2);

  private:
    int getRow(int site);
    int getColumn(int site);
    int getIndex(int column, int row);
    int m_width;
    int m_height;
  
};
  
} // End namespace Langmuir
#endif
