/*
 *  Grid - pure virtual base class to inherited by all grid classes
 *
 */

#ifndef __GRID_H
#define __GRID_H

#include <vector>

namespace Langmuir{

  class Grid {

  public:
    virtual ~Grid() { }

    /**
     * Get the nearest neighbours for the specified site.
     */
    virtual std::vector<int> getNeighbors(int site) = 0;

    /**
     * Sets the size of the grid.
     */
    virtual void setSize(int width, int height) = 0;
    
    /**
     * Gets the width of the grid.
     */
    virtual int getWidth() = 0;

    /**
     * Gets the height fo the grid.
     */
    virtual int getHeight() = 0;
    
    /**
     * Get the total distance between the two sites.
     */
    virtual double getTotalDistance(int site1, int site2) = 0;

    /**
     * Get the x component of the distance between the two sites.
     */
    virtual double getXDistance(int site1, int site2) = 0;

    /**
     * Get the x component of the distance between the two sites.
     */
    virtual double getYDistance(int site1, int site2) = 0;

  };

} // End namespace Langmuir

#endif
