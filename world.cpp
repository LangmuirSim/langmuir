#include "world.h"

#include "rand.h"

namespace Langmuir {

  World::World() : m_eField(0.0), m_grid(0), m_rand(new Rand(0.0, 1.0))
  {
    // This array is always the number of different sites + 2. The final two
    // rows/columns are for the source and drain site types.
    m_coupling.resize(4, 4);
    m_coupling << 0.333, 0.000, 0.333, 0.333,
                  0.000, 0.000, 0.000, 0.000,
                  0.333, 0.000, 0.002, 0.002,
                  0.333, 0.000, 0.002, 0.002;
    // This is the number of different site types and should add up to 1.0
    m_sitePercents.resize(2);
    m_sitePercents[0] = 0.95;
    m_sitePercents[1] = 0.05;
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
