/*
 *  Grid - pure virtual base class to inherited by all grid classes
 *
 */

#ifndef GRID_H
#define GRID_H

#include <vector>

namespace Langmuir
{

  class Agent;

  class Grid
  {

  public:
    virtual ~Grid()
    {
    }

    /**
     * Get the nearest neighbours for the specified site.
     */
    virtual std::vector<unsigned int> neighbors(unsigned int site) = 0;

    /**
     * Get the specified row.
     */
    virtual std::vector<unsigned int> row(unsigned int row) = 0;

    /**
     * Get the specified column.
     */
    virtual std::vector<unsigned int> col(unsigned int col) = 0;

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
     * Get the y component of the distance between the two sites.
     */
    virtual double getYDistance(unsigned int site1, unsigned int site2) = 0;

    /**
     * Convert a column/row into a one dimensional index.
     */
    virtual unsigned int getIndex(unsigned int column, unsigned int row) = 0;

    /**
     * Get the Agent at the specified site, or 0 if empty
     */
    virtual Agent * agent(unsigned int site, bool future = false) = 0;

    /**
     * Get the neighboring agents for the specified site.
     */
    virtual std::vector<Agent *> neighborAgents(unsigned int site, bool future = false) = 0;
  };

} // End namespace Langmuir

#endif
