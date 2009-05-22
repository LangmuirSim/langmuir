/*
 *  CubicGrid - cubic grid implementation of the Grid base class.
 *
 */

#ifndef CUBICGRID_H
#define CUBICGRID_H

#include "grid.h"

namespace Langmuir
{

  class CubicGrid : public Grid
  {

  public:

    CubicGrid(unsigned int width = 0, unsigned int height = 0);

    ~CubicGrid();

    /**
     * Get the 2D position vector of the specified site.
     */
    Eigen::Vector2d position(unsigned int site);

    /**
     * Get the nearest neighbours for the specified site.
     */
    std::vector<unsigned int> neighbors(unsigned int site);

    /**
     * Get the specified row.
     */
    std::vector<unsigned int> row(unsigned int row);

    /**
     * Get the specified column.
     */
    std::vector<unsigned int> col(unsigned int col);

    /**
     * Sets the size of the grid.
     */
    void setSize(unsigned int width, unsigned int height);

    /**
     * Gets the width of the grid.
     */
    unsigned int width();

    /**
     * Gets the height fo the grid.
     */
    unsigned int height();

    /**
     * Get the total distance between the two sites.
     */
    double getTotalDistance(unsigned int site1, unsigned int site2);

    /**
     * Get the x component of the distance between the two sites.
     */
    double getXDistance(unsigned int site1, unsigned int site2);

    /**
     * Get the y component of the distance between the two sites.
     */
    double getYDistance(unsigned int site1, unsigned int site2);

    /**
     * Convert a column/row into a one dimensional index.
     */
    unsigned int getIndex(unsigned int column, unsigned int row);

    /**
     * Get the neighboring agents for the specified site.
     */
    std::vector<Agent *> neighborAgents(unsigned int site);

    /**
     * Set the agent at the current site
     */
    void setAgent(unsigned int site, Agent *agent);

  protected:
    unsigned int getRow(unsigned int site);
    unsigned int getColumn(unsigned int site);
    unsigned int m_width;
    unsigned int m_height;

  };

  inline Eigen::Vector2d CubicGrid::position(unsigned int site)
  {
    if (site == static_cast<unsigned int>(-1)) return Eigen::Vector2d(-1,-1);
    return Eigen::Vector2d(0.5 + (site % m_width), 0.5 + int(site / m_width));
  }

} // End namespace Langmuir
#endif
