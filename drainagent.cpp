#include "drainagent.h"
#include "world.h"
#include "grid.h"
#include "inputparser.h"

namespace Langmuir
{

  DrainAgent::DrainAgent(World *world, int site) : Agent(Agent::Drain, world, site), m_acceptedCharges(0)
  {
      m_site = m_world->electronGrid()->volume() + 1;
      m_world->electronGrid()->setAgent(m_world->electronGrid()->volume() + 1,this);
      m_world->electronGrid()->setSiteID(m_world->electronGrid()->volume() + 1, 3);
      m_world->holeGrid()->setAgent(m_world->electronGrid()->volume() + 1,this);
      m_world->holeGrid()->setSiteID(m_world->electronGrid()->volume() + 1, 3);
      m_neighbors = m_world->electronGrid()->slice(
       m_world->electronGrid()->width()-1,m_world->electronGrid()->width(),
                                        0,m_world->electronGrid()->height(),
                                        0,m_world->electronGrid()->depth());
  }

  DrainAgent::~DrainAgent()
  {
  }

  bool DrainAgent::acceptCharge(int charge)
  {
    // The drain always accepts charges
    ++m_acceptedCharges; // Increment the counter
    return true;
  }

  int DrainAgent::charge()
  {
    // Always has a charge of 1
    return 1;
  }

  int DrainAgent::transport()
  {
    return 0;
  }

} // End Langmuir namespace
