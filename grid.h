#ifndef GRID_H
#define GRID_H

#include <vector>
#include <Eigen/Core>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

namespace Langmuir
{

  class Agent;

  /**
    *  @class Grid.
    *  @brief A grid to do simulations on.
    *
    *  Pure virtual base class to inherited by all grid classes.
    *  @date 06/07/2010
    */
  class Grid
  {

  public:

    /**
     * @brief virtual Destructor.
     */
    virtual ~Grid()
    {
    }

    /**
     * @brief neighborlist indices.
     *
     * Get a list of site indices for sites immediatly next to the given site.
     * @param site serial cell index.
     * @return neighbors list of serial site indices.
     */
    virtual QVector<int> neighbors(int site, int hoppingRange) = 0;

    /**
     * @brief rowlist.
     *
     * Get a list of site indices for all sites in a row of some given layer.
     * @param row row index.
     * @param layer layer index.
     * @return row list of row serial site indices.
     * @warning layer index optional.  default is layer zero.
     */
    virtual QVector<int> row(int row, int layer = 0) = 0;

    /**
     * @brief columnlist.
     *
     * Get a list of site indices for all sites in a column of some given layer.
     * @param col column index.
     * @param layer layer index.
     * @return column list of column serial site indices.
     * @warning layer index optional.  default is layer zero.
     */
    virtual QVector<int> col(int col, int layer = 0) = 0;

    /**
     * @brief resize the grid.
     *
     * Sets the values for width, depth, height, area, and volume.
     * Allocates memory for agents, potentials, and site serial indices.
     * @param width the number of columns
     * @param height the number of rows
     * @param depth the number of layers
     * @warning agents, potentials, and site serial indices are set to zero.
     */
    virtual void setSize(int width, int height, int depth) = 0;

    /**
     * @brief grid width.
     *
     * Data access to grid width.
     * @return width grid width.
     */
    virtual int width() = 0;

    /**
     * @brief grid height.
     *
     * Data access to grid height.
     * @return height grid height.
     */
    virtual int height() = 0;

    /**
     * @brief grid depth.
     *
     * Data access to grid depth.
     * @return depth grid depth.
     */
    virtual int depth() = 0;

    /**
     * @brief grid area.
     *
     * Data access to grid area.
     * @return area grid area.
     */
    virtual int area() = 0;

    /**
     * @brief grid volume.
     *
     * Data access to grid volume.
     * @return volume grid volume.
     */
    virtual int volume() = 0;

    /**
     * @brief site distance
     *
     * Get the floating total distance between the two sites using cell vector indices.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is double.
     */
    virtual double totalDistance(int site1, int site2) = 0;

    /**
     * @brief xdistance
     *
     * Get the floating x component of the distance between the two sites.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is double.
     */
    virtual double xDistance(int site1, int site2) = 0;

    /**
     * @brief ydistance
     *
     * Get the floating y component of the distance between the two sites.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is double.
     */
    virtual double yDistance(int site1, int site2) = 0;

    /**
     * @brief zdistance
     *
     * Get the floating z component of the distance between the two sites.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is double.
     */
    virtual double zDistance(int site1, int site2) = 0;

    /**
     * @brief xdistance
     *
     * Get the floating x component of the distance between the two sites.
     * The x-component of the second site is relfected through the yz-plane.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is double.
     */
    virtual double xImageDistance(int site1, int site2) = 0;

    /**
     * @brief ydistance
     *
     * Get the floating y component of the distance between the two sites.
     * The y-component of the second site is reflected through the xz-plane.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is double.
     */
    virtual double yImageDistance(int site1, int site2) = 0;

    /**
     * @brief zdistance
     *
     * Get the floating z component of the distance between the two sites.
     * The z-component of the second site is reflected through the xy-plane.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is double.
     */
    virtual double zImageDistance(int site1, int site2) = 0;

   /**
     * @brief integer xdistance
     *
     * Get the integer x component of the distance between the two sites.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is int.
     */
    virtual int xDistancei(int site1, int site2) = 0;

    /**
     * @brief integer ydistance
     *
     * Get the integer y component of the distance between the two sites.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is int.
     */
    virtual int yDistancei(int site1, int site2) = 0;

    /**
     * @brief integer zdistance
     *
     * Get the integer z component of the distance between the two sites.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is int.
     */
    virtual int zDistancei(int site1, int site2) = 0;

   /**
     * @brief integer xdistance
     *
     * Get the integer x component of the distance between the two sites.
     * The x-component of the second site is relfected through the yz-plane.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is int.
     */
    virtual int xImageDistancei(int site1, int site2) = 0;

    /**
     * @brief integer ydistance
     *
     * Get the integer y component of the distance between the two sites.
     * The y-component of the second site is reflected through the xz-plane.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is int.
     */
    virtual int yImageDistancei(int site1, int site2) = 0;

    /**
     * @brief integer zdistance
     *
     * Get the integer z component of the distance between the two sites.
     * The z-component of the second site is reflected through the xy-plane.
     * @param site1 serial cell index of site1.
     * @param site2 serial cell index of site2.
     * @return distance distance value is int.
     */
    virtual int zImageDistancei(int site1, int site2) = 0;

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
    virtual int getIndex(int column, int row, int layer = 0) = 0;

    /**
     * @brief Set site agent.
     *
     * Allow this site to point to a given agent.
     * @param site serial cell index.
     * @param agent pointer to an agent.
     * @warning unchecked access to agent list.
     */
    virtual void setAgent(int site, Agent *agent);

    /**
     * @brief get row.
     * 
     * Get the row index of a given site.
     * @param site serial cell index.
     * @return row row cell index.
     */
    virtual int getRow(int site) = 0;

    /**
     * @brief get column.
     * 
     * Get the column index of a given site.
     * @param site serial cell index.
     * @return column column cell index.
     */
    virtual int getColumn(int site) = 0;

    /**
     * @brief get layer.
     * 
     * Get the layer index of a given site.
     * @param site serial cell index.
     * @return layer layer cell index.
     */
    virtual int getLayer(int site) = 0;

    /**
     * @brief Get site agent.
     *
     * Get a pointer to an agent that this site is assigned to.
     * @param site serial cell index.
     * @return agent pointer to agent.
     * @warning unchecked access to agent list.
     */

    virtual Agent * agent(int site);

    /**
     * @brief Set site ID.
     *
     * Store this sites ID, i.e. this sites type, in a list.
     * @param site serial cell index.
     * @param site ID.
     * @warning unchecked access to site ID list.
     */
    virtual void setSiteID(int site, short id);

    /**
     * @brief Get site ID.
     *
     * Get this sites ID, i.e. this sites type.
     * @param site serial cell index.
     * @return ID site ID.
     * @warning unchecked access to site ID list.
     */
    virtual short siteID(int site);

    /**
     * @brief Set site potential.
     *
     * Store this sites potential in a list.
     * @param site serial cell index.
     * @param site potential.
     * @warning unchecked access to site ID list.
     */
    virtual void setPotential(int site, double potential);

    /**
     * @brief Get site potential.
     *
     * Get this sites potential.
     * @param site serial cell index.
     * @return potential site potential.
     * @warning unchecked access to site potential list.
     */
    virtual double potential(int site);

    /**
     * @brief Set source potential.
     *
     * Store the source potential in the potential array.
     * The location in the potential array is the number of grid sites.
     * @param site potential.
     */
    virtual void setSourcePotential(double potential);

    /**
     * @brief Set drain potential.
     *
     * Store the drain potential in the potential array.
     * The location in the potential array is the number of grid sites + 1.
     * @param site potential.
     */
    virtual void setDrainPotential(double potential);   

  protected:

    /**
     * @brief Site agent list.
     *
     * List of agent pointers.  Size is the same as the total number of sites.
     */
    std::vector<Agent *> m_agents;

    /**
     * @brief Site potential list.
     *
     * List of site potentials.  Size is the same as the total number of sites.
     */
    std::vector<double> m_potentials;

    /**
     * @brief Site type list.
     *
     * List of site IDs, i.e. the type of site.  Size is the same as the total number of sites.
     */
    std::vector<short> m_siteID;
  };

  inline void Grid::setAgent(int site, Agent *agent)
  {
    m_agents[site] = agent;
  }

  inline Agent * Grid::agent(int site)
  {
    return m_agents[site];
  }

  inline void Grid::setSiteID(int site, short id)
  {
    m_siteID[site] = id;
  }

  inline short Grid::siteID(int site)
  {
    return m_siteID[site];
  }

  inline void Grid::setPotential(int site, double potential)
  {
    m_potentials[site] = potential;
  }

  inline double Grid::potential(int site)
  {
    return m_potentials[site];
  }

  inline void Grid::setSourcePotential(double potential)
  {
    m_potentials[m_potentials.size()-2] = potential;
  }

  inline void Grid::setDrainPotential(double potential)
  {
    m_potentials[m_potentials.size()-1] = potential;
  }

} // End namespace Langmuir

#endif
