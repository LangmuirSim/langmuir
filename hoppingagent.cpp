#include "hoppingagent.h"

using namespace std;

namespace Langmuir{

  HoppingAgent::HoppingAgent()
  {
  }

  HoppingAgent::~HoppingAgent()
  {
  }

  void HoppingAgent::setNeighbors(std::vector<Agent *> neighbors)
  {
    m_neighbors = neighbors;
  }

  double HoppingAgent::potential()
  {
    return 0.;
  }

  bool HoppingAgent::acceptCharge(int charge)
  {
    return true;
  }

  int HoppingAgent::charge()
  {
    return 0;
  }

  bool HoppingAgent::transport()
  {
    return true;
  }
  
} // End Langmuir namespace
