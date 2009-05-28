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

    Eigen::MatrixXd * coupling();

  private:
    double m_eField;  // The electric field
    Logger *m_logger; // The data logger for our world (on this process)
    Grid *m_grid;     // The grid in use in the world
    Rand *m_rand;     // Random number generator
    QList<ChargeAgent *> m_charges; // Charge carriers in the system

    /**
     * Square matrix containing the coupling constants between different site types
     */
    Eigen::MatrixXd m_coupling; // Enumerates coupling constants between different sites

  };

  inline QList<ChargeAgent *> * World::charges()
  {
    return &m_charges;
  }

  inline Eigen::MatrixXd * World::coupling()
  {
    return &m_coupling;
  }

}

#endif // WORLD_H
