#include "sourceagent.h"

using namespace std;

namespace Langmuir{
  
  SourceAgent::SourceAgent() : m_potential(0.)
  {
  }
  
  SourceAgent::~SourceAgent()
  {
  }
  
  void HoppingAgent::setNeighbors(std::vector<Agent *> neighbors)
  {
    m_neighbors = neighbors;
  }
  
  double HoppingAgent::potential()
  {
    return m_potential;
  }
  
  bool HoppingAgent::acceptCharge(int charge)
  {
    // The source never accepts charges
    return false;
  }
  
  int HoppingAgent::charge()
  {
    // Always has a charge of 1
    return 1;
  }
  
  bool HoppingAgent::transport()
  {
    return true;
  }
  
} // End Langmuir namespace
