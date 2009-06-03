#include "sourceagent.h"

#include "world.h"
#include "grid.h"

#include <iostream>

namespace Langmuir
{
  using std::cout;
  using std::endl;

  SourceAgent::SourceAgent(World *world, unsigned int site, double potential) :
      Agent(Agent::Source, world, site), m_potential(potential), m_pBarrier(0.1),
      m_charges(0), m_maxCharges(100)
  {

  }

  SourceAgent::~SourceAgent()
  {
  }

  int SourceAgent::charge()
  {
    // Always has a charge of 1
    return 1;
  }

  unsigned int SourceAgent::transport()
  {
    // Determine whether a transport event will be attempted this tick
    // Used to maintain the average number of charges in the system
    if (m_charges >= m_maxCharges)
      return -1;

    // Generate a random number
    double rn = m_world->random();
    if (rn <= m_pBarrier)
      return -1;

    // Select a random neighbor and attempt transport.
    int irn = int(m_world->random() * double(m_neighbors.size()-0.00000001));

    // Use another random number to determine whether the charge is injected
    if (!m_world->grid()->agent(m_neighbors[irn])) {
      ++m_charges;
      return m_neighbors[irn];
    }

    return -1;
  }

} // End Langmuir namespace
