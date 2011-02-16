/**
 * World - acts as a container for global information in the simulation
 */
#ifndef WORLD_H
#define WORLD_H

#include <vector>
#include <Eigen/Core>
#include <QtCore/QList>

namespace Langmuir
{
  class Grid;
  class Rand;
  class ChargeAgent;

  /**
    * @class Triple Index Array
    * @brief Three dimensional matrix
    *
    * For storing doubles to be accessed by col, row, and lay indices.
    */
  class TripleIndexArray
  {
   public:
    /**
      * @brief Default constructor.
      *
      * Sets everything to zero.
      */
    TripleIndexArray ();

    /**
      * @brief Alternate constructor.
      *
      * Sets columns, rows, layers, area, and width to indicated values. 
      * Resizes the internal vector to hold the appropriate number of values.
      */
    TripleIndexArray (unsigned int col, unsigned int row, unsigned int lay);

    /** 
      * @brief operator overload
      *
      * Access a double at col, row, lay position.
      */
    double& operator() (unsigned int col, unsigned int row, unsigned int lay);

    /** 
      * @brief operator overload 
      *
      * Access a double at col, row, lay position.
      */
    double operator() (unsigned int col, unsigned int row, unsigned int lay) const;

    /**
      * @brief Clear values and reallocate space.
      *
      * Sets columns, rows, layers, area, and width to indicated values. 
      * Resizes the internal vector to hold the appropriate number of values.
      */
    void resize (unsigned int col, unsigned int row, unsigned int lay);

    /**
      * @brief member access.
      *
      * Get the number of rows. 
      */
    inline unsigned int rows() const { return m_row; }

    /**
      * @brief member access.
      *
      * Get the number of columns.
      */
    inline unsigned int cols() const { return m_col; }

    /**
      * @brief member access.
      *
      * Get the number of layerss.
      */
    inline unsigned int lays() const { return m_lay; }

    /**
      * @brief member access.
      *
      * Get the number of values.
      */
    inline unsigned int size() const { return values.size(); }

   private:

    /**
      * @brief member.
      *
      * Array of doubles.
      */
    std::vector<double> values;

    /**
      * @brief member.
      *
      * The number of columns.
      */
    unsigned int m_width;

    /**
      * @brief member.
      *
      * The number of columns times the number of rows.
      * The total number of items in a layer.
      */
    unsigned int m_area;

    /**
      * @brief member.
      *
      * The number of columns.
      */
    unsigned int m_col;

    /**
      * @brief member.
      *
      * The number of rows.
      */
    unsigned int m_row;

    /**
      * @brief member.
      *
      * The number of layers.
      */
    unsigned int m_lay;
  };

  const unsigned int errorValue = -1;

  /**
    * @class World
    *
    * Holds information for simulation.
    */
  class World
  {
   public:
    /**
      * Default Constructor.
      */
    World();

    /**
      * Default Destructor.
      */
   ~World();

    /**
      * @brief member access.
      *
      * Get the address of the grid object.
      */
    Grid * grid() const { return m_grid; }

    /**
      * @brief member access.
      *
      * Set the address of the grid object.
      */
    void setGrid(Grid *grid) { m_grid = grid; }

    /**
      * @brief random number generation.
      *
      * Get a random number between 0 and 1.
      */
    double random();

    /**
      * @brief random number generation.
      *
      * Get a random number with average and standard deviation of gaussian.
      * @param average average of gaussian
      * @param stdev standard deviation of gaussian
      */
    double random( double average, double stdev );

    /**
      * @brief member access.
      *
      * Get the address of a list of charge agents.
      */
    QList<ChargeAgent *> * charges();

    /**
      * @brief member access.
      *
      * Get the address of a list of charged defects.
      */
    QList<unsigned int> * chargedDefects();

    /**
      * @brief member access.
      *
      * Get the address of a list of charged traps.
      */
    QList<unsigned int> * chargedTraps();

    /**
      * @brief member access.
      *
      * Get the address of a list of coupling constants.
      */
    Eigen::MatrixXd * coupling();

    /**
      * @brief member access.
      *
      * Get a reference to a distance matrix. 
      */
    TripleIndexArray& interactionEnergies();

   private:
    Grid                 *m_grid;                // The grid in use in the world
    Rand                 *m_rand;                // Random number generator
    QList<ChargeAgent *>  m_charges;             // Charge carriers in the system
    QList<unsigned int>   m_chargedDefects;      // Charged defects in the system
    QList<unsigned int>   m_chargedTraps;        // Charged traps in the system
    Eigen::MatrixXd       m_coupling;            // Enumerates coupling constants between different sites
    std::vector<double>   m_sitePercents;        // Percentage of each site in the grid
    TripleIndexArray      m_interactionEnergies; // Interaction energies
 };

 inline QList<ChargeAgent *> * World::charges()
 {
  return &m_charges;
 }

 inline QList<unsigned int> * World::chargedDefects()
 {
  return &m_chargedDefects;
 }

 inline QList<unsigned int> * World::chargedTraps()
 {
  return &m_chargedTraps;
 }

 inline Eigen::MatrixXd * World::coupling()
 {
  return &m_coupling;
 }

 inline TripleIndexArray& World::interactionEnergies()
 {
  return m_interactionEnergies;
 }

}

#endif // WORLD_H
