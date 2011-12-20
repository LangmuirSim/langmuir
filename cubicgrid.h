#ifndef CUBICGRID_H
#define CUBICGRID_H

#include "grid.h"

namespace Langmuir
{
  /**
    *  @class CubicGrid
    *  @brief A Grid to hold site types and site potentials
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
    CubicGrid( int width = 0, int height = 0, int depth = 1 );

    /**
     * @brief Destructor.
     */
    ~CubicGrid();

    /**
     * @brief rowlist.
     *
     * Get a list of site indices for all sites in a row of some given layer.
     * @param row row index.
     * @param layer layer index.
     * @return row list of row serial site indices.
     * @warning layer index optional.  default is layer zero.
     */
    QVector<int> row(int row, int layer = 0);

    /**
     * @brief columnlist.
     *
     * Get a list of site indices for all sites in a column of some given layer.
     * @param col column index.
     * @param layer layer index.
     * @return column list of column serial site indices.
     * @warning layer index optional.  default is layer zero.
     */
    QVector<int> col(int col, int layer = 0);

    /**
     * @brief grid width.
     *
     * Data access to grid width.
     * @return width grid width.
     */
    int width();

    /**
     * @brief grid height.
     *
     * Data access to grid height.
     * @return height grid height.
     */
    int height();

    /**
     * @brief grid depth.
     *
     * Data access to grid depth.
     * @return depth grid depth.
     */
    int depth();

    /**
     * @brief grid area.
     *
     * Data access to grid area.
     * @return area grid area.
     */
    int area();

    /**
     * @brief grid volume.
     *
     * Data access to grid volume.
     * @return volume grid volume.
     */
    int volume();

    /**
     * @brief site distance
     *
     * Get the floating total distance between the two sites using cell vector indices.  
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is double.
     */
    double totalDistance(int site1, int site2);

    /**
     * @brief xdistance
     *
     * Get the floating x component of the distance between the two sites.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is double.   
     */
    double xDistance(int site1, int site2);

    /**
     * @brief ydistance
     *
     * Get the floating y component of the distance between the two sites.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is double.
     */
    double yDistance(int site1, int site2);

    /**
     * @brief zdistance
     *
     * Get the floating z component of the distance between the two sites.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is double.
     */
    double zDistance(int site1, int site2);

    /**
     * @brief xdistance
     *
     * Get the floating x component of the distance between the two sites.
     * The x-component of the second site is relfected through the yz-plane.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is double.
     */
    double xImageDistance(int site1, int site2);

    /**
     * @brief ydistance
     *
     * Get the floating y component of the distance between the two sites.
     * The y-component of the second site is reflected through the xz-plane.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is double.
     */
    double yImageDistance(int site1, int site2);

    /**
     * @brief zdistance
     *
     * Get the floating z component of the distance between the two sites.
     * The z-component of the second site is reflected through the xy-plane.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is double.
     */
    double zImageDistance(int site1, int site2);

    /**
     * @brief integer xdistance
     *
     * Get the integer x component of the distance between the two sites.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is int.
     */
    int xDistancei(int site1, int site2);

    /**
     * @brief integer ydistance
     *
     * Get the integer y component of the distance between the two sites.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is int.
     */
    int yDistancei(int site1, int site2);

    /**
     * @brief integer zdistance
     *
     * Get the integer z component of the distance between the two sites.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is int.
     */
    int zDistancei(int site1, int site2);

   /**
     * @brief integer xdistance
     *
     * Get the integer x component of the distance between the two sites.
     * The x-component of the second site is relfected through the yz-plane.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is int.
     */
    int xImageDistancei(int site1, int site2);

    /**
     * @brief integer ydistance
     *
     * Get the integer y component of the distance between the two sites.
     * The y-component of the second site is reflected through the xz-plane.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is int.
     */
    int yImageDistancei(int site1, int site2);

    /**
     * @brief integer zdistance
     *
     * Get the integer z component of the distance between the two sites.
     * The z-component of the second site is reflected through the xy-plane.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is int.
     */
    int zImageDistancei(int site1, int site2);

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
    int getIndex( int column, int row, int layer = 0 );

    /**
     * @brief set agent.
     *
     * Allow this site to point to a given agent.
     * @param site serial cell index.
     * @param agent pointer to an agent.
     * @warning unchecked access to agent list.
     */
    void setAgentAddress(int site, Agent *agentAddress);

    /**
     * @brief get row.
     * 
     * Get the row index of a given site.
     * @param site serial cell index.
     * @return row row cell index.
     */
    int getRow(int site);

    /**
     * @brief get column.
     * 
     * Get the column index of a given site.
     * @param site serial cell index.
     * @return column column cell index.
     */
    int getColumn(int site);

    /**
     * @brief get layer.
     * 
     * Get the layer index of a given site.
     * @param site serial cell index.
     * @return layer layer cell index.
     */
    int getLayer(int site);

    /**
     * @brief Create a list of site ids.
     *
     * Generate the neighbors of a given site
     */
    QVector<int> neighborsSite(int site);

    /**
     * @brief Create a list of site ids.
     *
     * Generate the site ids of a slice of the grid
     */
    QVector<int> sliceIndex(int xi, int xf, int yi, int yf, int zi, int zf);

  protected:
    /**
     * @brief grid width.
     *
     * The number of columns in the grid.
     */
    int m_width;

    /**
     * @brief grid height.
     *
     * The number of rows in the grid.
     */
    int m_height;

    /**
     * @brief grid depth.
     *
     * The number of layers in the grid.
     */
    int m_depth;

    /**
     * @brief grid area.
     *
     * The number of sites in a layer.
     */
    int m_area;

    /**
     * @brief grid volume.
     *
     * The total number of sites.
     */
    int m_volume;
  };
} // End namespace Langmuir
#endif
