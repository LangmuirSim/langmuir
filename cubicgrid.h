#ifndef CUBICGRID_H
#define CUBICGRID_H

#include "grid.h"
#include <QtCore/QFile>
#include <QtCore/QTextStream>

namespace Langmuir
{

  /**
    *  @class Cubicgrid
    *  @brief A grid to do simulations on.
    *
    *  Three dimensional cubic grid implementation of the Grid base class.
    *  @date 06/07/2010
    */
  class CubicGrid : public Grid
  {

  public:
    
    /**
     * @brief Default Constructor.
     *
     * Creates a cubic grid of dimensions width x height x depth.
     * Sets the values for width, depth, height, area, and volume.
     * Allocates memory for agents, potentials, and site serial indices. 
     * @param width the number of columns.
     * @param height the number of rows.
     * @param depth the number of layers.
     * @warning agents, potentials, and site serial indices are set to zero.
     * @warning depth value is optional.  default depth is one.
     */
    CubicGrid( unsigned int width = 0, unsigned int height = 0, unsigned int depth = 1 );

    /**
     * @brief Destructor.
     */
    ~CubicGrid();

    /**
     * @brief position vector.
     *
     * Get a vector pointing from the origin to the middle of a cell.
     * @param site serial cell index.
     */
    Eigen::Vector3d position(unsigned int site);

    /**
     * @brief neighborlist indices.
     *
     * Get a list of site indices for sites immediatly next to the given site.
     * @param site serial cell index.
     * @return neighbors list of serial site indices.
     */
    std::vector<unsigned int> neighbors(unsigned int site);

    /**
     * @brief rowlist.
     *
     * Get a list of site indices for all sites in a row of some given layer.
     * @param row row index.
     * @param layer layer index.
     * @return row list of row serial site indices.
     * @warning layer index optional.  default is layer zero.
     */
    std::vector<unsigned int> row(unsigned int row, unsigned int layer = 0);

    /**
     * @brief columnlist.
     *
     * Get a list of site indices for all sites in a column of some given layer.
     * @param col column index.
     * @param layer layer index.
     * @return column list of column serial site indices.
     * @warning layer index optional.  default is layer zero.
     */
    std::vector<unsigned int> col(unsigned int col, unsigned int layer = 0);

    /**
     * @brief resize the grid.
     *
     * Creates a cubic grid of dimensions width x height x depth.
     * Sets the values for width, depth, height, area, and volume.
     * Allocates memory for agents, potentials, and site serial indices.
     * @param width the number of columns
     * @param height the number of rows
     * @param depth the number of layers
     * @warning agents, potentials, and site serial indices are set to zero.
     */
    void setSize(unsigned int width, unsigned int height, unsigned int depth);

    /**
     * @brief grid width.
     *
     * Data access to grid width.
     * @return width grid width.
     */
    unsigned int width();

    /**
     * @brief grid height.
     *
     * Data access to grid height.
     * @return height grid height.
     */
    unsigned int height();

    /**
     * @brief grid depth.
     *
     * Data access to grid depth.
     * @return depth grid depth.
     */
    unsigned int depth();

    /**
     * @brief grid area.
     *
     * Data access to grid area.
     * @return area grid area.
     */
    unsigned int area();

    /**
     * @brief grid volume.
     *
     * Data access to grid volume.
     * @return volume grid volume.
     */
    unsigned int volume();

    /**
     * @brief site distance
     *
     * Get the floating total distance between the two sites using cell vector indices.  
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is double.
     */
    double totalDistance(unsigned int site1, unsigned int site2);

    /**
     * @brief xdistance
     *
     * Get the floating x component of the distance between the two sites.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is double.   
     */
    double xDistance(unsigned int site1, unsigned int site2);

    /**
     * @brief ydistance
     *
     * Get the floating y component of the distance between the two sites.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is double.
     */
    double yDistance(unsigned int site1, unsigned int site2);

    /**
     * @brief zdistance
     *
     * Get the floating z component of the distance between the two sites.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is double.
     */
    double zDistance(unsigned int site1, unsigned int site2);

    /**
     * @brief integer xdistance
     *
     * Get the integer x component of the distance between the two sites.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is int.
     */
    int xDistancei(unsigned int site1, unsigned int site2);

    /**
     * @brief integer ydistance
     *
     * Get the integer y component of the distance between the two sites.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is int.
     */
    int yDistancei(unsigned int site1, unsigned int site2);

    /**
     * @brief integer zdistance
     *
     * Get the integer z component of the distance between the two sites.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is int.
     */
    int zDistancei(unsigned int site1, unsigned int site2);

    /**
     * @brief serial cell index
     *
     * Get the serial index of a site whose vector index is column, row, layer.
     * @param col column index of site.
     * @param row row index of site.
     * @param layer layer index of site.
     * @return integer serial cell index.
     * @warning layer index optional.  default is layer zero.
     */
    unsigned int getIndex( unsigned int column, unsigned int row, unsigned int layer = 0 );

    /**
     * @brief neighborlist agents.
     *
     * Get a list of neightboring agent pointers for sites immediatly next to the given site.
     * @param site serial cell index.
     * @return neighbors list of agent pointers.
     */
    std::vector<Agent *> neighborAgents(unsigned int site);

    /**
     * @brief set agent.
     *
     * Allow this site to point to a given agent.
     * @param site serial cell index.
     * @param agent pointer to an agent.
     * @warning unchecked access to agent list.
     */
    void setAgent(unsigned int site, Agent *agent);

    /**
     * @brief get row.
     * 
     * Get the row index of a given site.
     * @param site serial cell index.
     * @return row row cell index.
     */
    unsigned int getRow(unsigned int site);

    /**
     * @brief get column.
     * 
     * Get the column index of a given site.
     * @param site serial cell index.
     * @return column column cell index.
     */
    unsigned int getColumn(unsigned int site);

    /**
     * @brief get layer.
     * 
     * Get the layer index of a given site.
     * @param site serial cell index.
     * @return layer layer cell index.
     */
    unsigned int getLayer(unsigned int site);

    /**
     * @brief print XYZ.
     *
     * Write out positions of agent.
     */
    void print3D( QTextStream& stream );

  protected:
    /**
     * @brief grid width.
     *
     * The number of columns in the grid.
     */
    unsigned int m_width;

    /**
     * @brief grid height.
     *
     * The number of rows in the grid.
     */
    unsigned int m_height;

    /**
     * @brief grid depth.
     *
     * The number of layers in the grid.
     */
    unsigned int m_depth;

    /**
     * @brief grid area.
     *
     * The number of sites in a layer.
     */
    unsigned int m_area;

    /**
     * @brief grid volume.
     *
     * The total number of sites.
     */
    unsigned int m_volume;
  };

  inline Eigen::Vector3d CubicGrid::position(unsigned int site)
  {
    if (site == static_cast<unsigned int>(-1)) return Eigen::Vector3d(-1,-1,-1);
    return Eigen::Vector3d(0.5 + (site % m_width), 0.5 + int( site / m_width - ( site / m_area ) * m_height ), 0.5 + getLayer(site) );
  }

} // End namespace Langmuir
#endif
