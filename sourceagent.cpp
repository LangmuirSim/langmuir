#include "sourceagent.h"

using namespace std;

namespace Langmuir{
  
  SourceAgent::SourceAgent() : m_potential(0.)
  {
  }
  
  SourceAgent::~SourceAgent()
  {
  }
  
  void SourceAgent::setNeighbors(std::vector<Agent *> neighbors)
  {
    m_neighbors = neighbors;
  }
  
  double SourceAgent::potential()
  {
    return m_potential;
  }
  
  bool SourceAgent::acceptCharge(int charge)
  {
    // The source never accepts charges
    return false;
  }
  
  int SourceAgent::charge()
  {
    // Always has a charge of 1
    return 1;
  }
  
  bool SourceAgent::transport()
  {
    return true;
  }
  
} // End Langmuir namespace
