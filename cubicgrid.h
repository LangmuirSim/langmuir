/*
 *  CubicGrid - cubic grid implementation of the Grid base class.
 *
 */

#ifndef __CUBICGRID_H
#define __CUBICGRID_H

#include "grid.h"

#include <vector>

namespace Langmuir
{

class CubicGrid : public Grid
{

public:

	CubicGrid();

	CubicGrid(unsigned int width, unsigned int height);

	~CubicGrid();

	/**
	 * Get the nearest neighbours for the specified site.
	 */
	std::vector<unsigned int> getNeighbors(unsigned int site);

	/**
	 * Get the specified row.
	 */
	virtual std::vector<unsigned int> row(unsigned int row);

	/**
	 * Get the specified column.
	 */
	virtual std::vector<unsigned int> col(unsigned int col);

	/**
	 * Sets the size of the grid.
	 */
	void setSize(unsigned int width, unsigned int height);

	/**
	 * Gets the width of the grid.
	 */
	unsigned int getWidth();

	/**
	 * Gets the height fo the grid.
	 */
	unsigned int getHeight();

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

private:
	unsigned int getRow(unsigned int site);
	unsigned int getColumn(unsigned int site);
	unsigned int m_width;
	unsigned int m_height;

};

} // End namespace Langmuir
#endif
