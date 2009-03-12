/**
 * World - acts as a container for global information in the simulation
 */

#ifndef WORLD_H
#define WORLD_H

namespace Langmuir
{

  class Grid;
  class Rand;

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

  private:
    double m_eField;  // The electric field
    Grid *m_grid;     // The grid in use in the world
    Rand *m_rand;     // Random number generator

  };

}

#endif // WORLD_H
