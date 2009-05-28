#include "world.h"

#include "rand.h"

namespace Langmuir {

  World::World() : m_eField(0.0), m_grid(0), m_rand(new Rand(0.0, 1.0))
  {
    m_coupling.resize(4, 4);
    m_coupling << 0.333, 0.002, 0.333, 0.333,
                  0.002, 0.001, 0.002, 0.002,
                  0.333, 0.002, 0.002, 0.002,
                  0.333, 0.002, 0.002, 0.002;
  }

  World::~World()
  {
    delete m_rand;
  }

  double World::random()
  {
    return m_rand->number();
  }

}
