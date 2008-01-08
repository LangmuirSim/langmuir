#include "drainagent.h"

using namespace std;

namespace Langmuir{
  
  DrainAgent::DrainAgent() : m_potential(0.)
  {
  }
  
  DrainAgent::~DrainAgent()
  {
  }
  
  void DrainAgent::setNeighbors(std::vector<Agent *> neighbors)
  {
    m_neighbors = neighbors;
  }
  
  double DrainAgent::potential()
  {
    return m_potential;
  }
  
  bool DrainAgent::acceptCharge(int charge)
  {
    // The source never accepts charges
    return false;
  }
  
  int DrainAgent::charge()
  {
    // Always has a charge of 1
    return 1;
  }
  
  bool DrainAgent::transport()
  {
    return true;
  }
  
} // End Langmuir namespace
