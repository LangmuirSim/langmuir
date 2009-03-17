#include "drainagent.h"

using namespace std;

namespace Langmuir
{

  DrainAgent::DrainAgent(World *world, unsigned int site, double potential) :
      Agent(world, site), m_potential(potential)
  {
  }

  DrainAgent::~DrainAgent()
  {
  }

  bool DrainAgent::acceptCharge(int charge)
  {
    // The source never accepts charges
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
