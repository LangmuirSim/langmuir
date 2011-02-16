#include "drainagent.h"
#include "world.h"
#include "grid.h"
#include "inputparser.h"

using namespace std;

namespace Langmuir
{

  DrainAgent::DrainAgent(World *world, unsigned int site) : Agent(Agent::Drain, world, site), m_acceptedCharges(0)
  {
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

  unsigned int DrainAgent::transport()
  {
    return 0;
  }

} // End Langmuir namespace
