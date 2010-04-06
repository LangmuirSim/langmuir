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
  class Logger;
  class Grid;
  class Rand;
  class ChargeAgent;

  const unsigned int errorValue = -1;

  class World
  {
  public:
    World();
    ~World();

    double eField() const { return m_eField; }
    void setEField(double field) { m_eField = field; }

    Grid * grid() const { return m_grid; }
    void setGrid(Grid *grid) { m_grid = grid; }

    /**
     * Random number between 0.0 and 1.0.
     */
    double random();

    QList<ChargeAgent *> * charges();
	  
	QList<unsigned int> * chargedDefects();

    Eigen::MatrixXd * coupling();

    Eigen::MatrixXd * interactionEnergies();

  private:
    double m_eField;  // The electric field
    Logger *m_logger; // The data logger for our world (on this process)
    Grid *m_grid;     // The grid in use in the world
    Rand *m_rand;     // Random number generator
    QList<ChargeAgent *> m_charges; // Charge carriers in the system
	QList<unsigned int> m_chargedDefects; // Charged defects in the system

    /**
     * Square matrix containing the coupling constants between different site types
     */
    Eigen::MatrixXd m_coupling; // Enumerates coupling constants between different sites
    std::vector<double> m_sitePercents; // Percentage of each site in the grid
    Eigen::MatrixXd m_interactionEnergies; // Interaction energies

  };

  inline QList<ChargeAgent *> * World::charges()
  {
    return &m_charges;
  }
	
  inline QList<unsigned int> * World::chargedDefects()
  {
	return &m_chargedDefects;
  }

  inline Eigen::MatrixXd * World::coupling()
  {
    return &m_coupling;
  }

  inline Eigen::MatrixXd * World::interactionEnergies()
  {
    return &m_interactionEnergies;
  }

}

#endif // WORLD_H
