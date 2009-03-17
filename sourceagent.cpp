#include "sourceagent.h"

#include "world.h"
#include "grid.h"

#include <iostream>

namespace Langmuir
{
  using std::cout;
  using std::endl;

  SourceAgent::SourceAgent(World *world, unsigned int site, double potential) :
      Agent(world, site), m_potential(potential), m_pBarrier(0.1)
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
    double rn = m_world->random();
    cout << "Source transport() called " << rn << endl;;
    if (rn <= m_pBarrier) return -1;

    // This is currently limiting the number of charges that can be injected
    if (m_charges > 300) return -1;

    // Select a random neighbor and attempt transport.
    int irn = int(m_world->random() * double(m_neighbors.size()));
//    cout << "Source attempt - neighbor type " << m_world->grid()->siteID(m_neighbors[irn])
//         << " site number " << m_neighbors[irn] << " " << irn
//         << " " << m_world->grid()->agent(m_neighbors[irn]) << endl;
    // Use another random number to determine whether the charge is injected
    if (!m_world->grid()->agent(m_neighbors[irn]) && m_world->random() > 0.1) {
      cout << "Charge injected! " << m_neighbors[irn];
      ++m_charges;
      return m_neighbors[irn];
    }
    return -1;
  }

} // End Langmuir namespace
