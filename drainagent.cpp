#include "drainagent.h"
#include "world.h"
#include "grid.h"
#include "inputparser.h"

namespace Langmuir
{

  DrainAgent::DrainAgent(World *world, int site) : Agent(Agent::Drain, world, site), m_acceptedCharges(0)
  {
      m_site = m_world->grid()->volume() + 1;
      m_world->grid()->setAgent(m_world->grid()->volume() + 1,this);
      m_world->grid()->setSiteID(m_world->grid()->volume() + 1, 3);
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
