/*
 *  Grid - pure virtual base class to inherited by all grid classes
 *
 */

#ifndef GRID_H
#define GRID_H

#include <vector>

#include <Eigen/Core>

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
     * Get the 2D position vector of the specified site.
     */
    virtual Eigen::Vector2d position(unsigned int site) = 0;

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
    virtual unsigned int width() = 0;

    /**
     * Gets the height fo the grid.
     */
    virtual unsigned int height() = 0;

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
     * Set the agent at the current site
     */
    virtual void setAgent(unsigned int site, Agent *agent);

    /**
     * Get the Agent at the specified site, or 0 if empty
     */
    virtual Agent * agent(unsigned int site);

    /**
     * Set the agent at the current site
     */
    virtual void setSiteID(unsigned int site, short id);

    /**
     * Get the Agent at the specified site, or 0 if empty
     */
    virtual short siteID(unsigned int site);

    /**
     * Set the potential at the site.
     */
    virtual void setPotential(unsigned int site, double potential);

    /**
     * Get the site potential.
     */
    virtual double potential(unsigned int site);

    /**
     * Get the neighboring agents for the specified site.
     */
    virtual std::vector<Agent *> neighborAgents(unsigned int site) = 0;

  protected:
    std::vector<Agent *> m_agents;
    std::vector<double> m_potentials;
    std::vector<short> m_siteID;  // The ID of the site at the index
  };

  inline void Grid::setAgent(unsigned int site, Agent *agent)
  {
    m_agents[site] = agent;
  }

  inline Agent * Grid::agent(unsigned int site)
  {
    // No range checking, etc
    return m_agents[site];
  }

  inline void Grid::setSiteID(unsigned int site, short id)
  {
    m_siteID[site] = id;
  }

  inline short Grid::siteID(unsigned int site)
  {
    // No range checking, etc
    return m_siteID[site];
  }

  inline void Grid::setPotential(unsigned int site, double potential)
  {
    m_potentials[site] = potential;
  }

  inline double Grid::potential(unsigned int site)
  {
    return m_potentials[site];
  }

} // End namespace Langmuir

#endif
