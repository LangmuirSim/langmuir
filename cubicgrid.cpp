#include "cubicgrid.h"

#include <cmath>

using namespace std;

namespace Langmuir
{

CubicGrid::CubicGrid() :
	m_width(0), m_height(0)
{
}

CubicGrid::CubicGrid(unsigned int width, unsigned int height) :
	m_width(width), m_height(height)
{
}

CubicGrid::~CubicGrid()
{

}

vector<unsigned int> CubicGrid::getNeighbors(unsigned int site)
{
	// Return the indexes of all nearest neighbours
	vector<unsigned int> neighbors;
	unsigned int column = getColumn(site);
	unsigned int row = getRow(site);
	// To the left
	if (column > 0)
		neighbors.push_back(getIndex(column-1, row));
	// To the right
	if (column < m_width - 1)
		neighbors.push_back(getIndex(column+1, row));
	// Up
	if (row > 0)
		neighbors.push_back(getIndex(column, row-1));
	// Down
	if (row < m_height - 1)
		neighbors.push_back(getIndex(column, row+1));

	// Now for the source and the drain....
	if (column == 0)
		neighbors.push_back(getIndex(m_width, m_height) + 1);
	if (column == m_width - 1)
		neighbors.push_back(getIndex(m_width, m_height) + 2);

	// Now we have all the nearest neighbours - between 2 and 4 in this case
	return neighbors;
}

vector<unsigned int> CubicGrid::row(unsigned int row)
{
	vector<unsigned int> vRow;
	for (unsigned int i = 0; i < m_width; i++)
	{
		vRow.push_back(getIndex(i, row));
	}
	return vRow;
}

vector<unsigned int> CubicGrid::col(unsigned int col)
{
	vector<unsigned int> vCol;
	for (unsigned int i = 0; i < m_height; i++)
	{
		vCol.push_back(getIndex(col, i));
	}
	return vCol;
}

void CubicGrid::setSize(unsigned int width, unsigned int height)
{
	m_width = width;
	m_height = height;
}

unsigned int CubicGrid::getWidth()
{
	return m_width;
}

unsigned int CubicGrid::getHeight()
{
	return m_height;
}

double CubicGrid::getTotalDistance(unsigned int site1, unsigned int site2)
{
	return sqrt((getRow(site1) - getRow(site2)) * (getRow(site1)
			- getRow(site2)) + (getColumn(site1) - getColumn(site2))
			* (getColumn(site1) - getColumn(site2)));
}

double CubicGrid::getXDistance(unsigned int site1, unsigned int site2)
{
	return fabs(int(getRow(site1)) - int(getRow(site2)));
}

double CubicGrid::getYDistance(unsigned int site1, unsigned int site2)
{
	return fabs(int(getColumn(site1)) - int(getColumn(site2)));
}

unsigned int CubicGrid::getRow(unsigned int site)
{
	return site / m_width;
}

unsigned int CubicGrid::getColumn(unsigned int site)
{
	return site % m_width;
}

unsigned int CubicGrid::getIndex(unsigned int column, unsigned int row)
{
	return m_width * row + column;
}

} // End namespace Langmuir
