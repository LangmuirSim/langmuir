#include "hoppingagent.h"

#include "rand.h"

#include <cmath>
#include <iostream>

using namespace std;

namespace Langmuir
{

HoppingAgent::HoppingAgent(unsigned int site, const Grid* grid) : Agent(site),
    m_grid(grid), m_charge(0), m_fCharge(0),
    m_rand(new Rand(0.0, 1.0)), m_pBarrier(0.4)
{
}

HoppingAgent::~HoppingAgent()
{
    delete m_rand;
    m_rand = 0;
}

void HoppingAgent::setNeighbors(std::vector<Agent *> neighbors)
{
  m_neighbors = neighbors;
}

bool HoppingAgent::acceptCharge(int charge)
{
  // Can only accept a charge if we are currently vacant and have not already
  // accepted a charge from another neighbour.
  if (m_charge == 0 && m_fCharge == 0) {
    m_fCharge = charge;
    return true;
  }
  else
    return false;
}

int HoppingAgent::charge()
{
  return m_charge;
}

int HoppingAgent::fCharge()
{
  return m_fCharge;
}

double HoppingAgent::pBarrier()
{
  return m_pBarrier;
}

void HoppingAgent::setPBarrier(double pBarrier)
{
  m_pBarrier = pBarrier;
}

Agent* HoppingAgent::transport()
{
    // Determine whether a transport event will be attempted this tick.
    if (m_charge == 0)
        return 0;

    return 0;
}

void HoppingAgent::completeTick()
{
    // The future state becomes the current state.
    m_charge = m_fCharge;
}

} // End Langmuir namespace
