/**
 * World - acts as a container for global information in the simulation
 */

#ifndef WORLD_H
#define WORLD_H

#include <vector>

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

    std::vector<ChargeAgent *> * charges();

  private:
    double m_eField;  // The electric field
    Logger *m_logger; // The data logger for our world (on this process)
    Grid *m_grid;     // The grid in use in the world
    Rand *m_rand;     // Random number generator
    std::vector<ChargeAgent *> m_charges; // Charge carriers in the system

  };

  inline std::vector<ChargeAgent *> * World::charges()
  {
    return &m_charges;
  }

}

#endif // WORLD_H
