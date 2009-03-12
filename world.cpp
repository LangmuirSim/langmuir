#include "world.h"

#include "rand.h"

namespace Langmuir {

  World::World() : m_eField(0.0), m_grid(0), m_rand(new Rand(0.0, 1.0))
  {
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
