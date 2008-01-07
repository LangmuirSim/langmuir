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
    virtual std::vector<unsigned int> getNeighbors(unsigned int site) = 0;

    /**
     * Sets the size of the grid.
     */
    virtual void setSize(unsigned int width, unsigned int height) = 0;
    
    /**
     * Gets the width of the grid.
     */
    virtual unsigned int getWidth() = 0;

    /**
     * Gets the height fo the grid.
     */
    virtual unsigned int getHeight() = 0;
    
    /**
     * Get the total distance between the two sites.
     */
    virtual double getTotalDistance(unsigned int site1, unsigned int site2) = 0;

    /**
     * Get the x component of the distance between the two sites.
     */
    virtual double getXDistance(unsigned int site1, unsigned int site2) = 0;

    /**
     * Get the x component of the distance between the two sites.
     */
    virtual double getYDistance(unsigned int site1, unsigned int site2) = 0;

  };

} // End namespace Langmuir

#endif
