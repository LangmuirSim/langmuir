#include "hoppingagent.h"

#include "rand.h"

#include <cmath>
#include <iostream>

using namespace std;

namespace Langmuir
{

HoppingAgent::HoppingAgent(unsigned int site, const Grid* grid) :
    m_site(site), m_grid(grid), m_charge(0), m_fCharge(0),
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

double HoppingAgent::potential()
{
    return m_potential;
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

    // If the pBarrier is not overcome then no transport is attempted.
    double rn = m_rand->number();
    if (rn <= m_pBarrier) return 0;

    // Get the potential of each neighbour, then work out its Boltzmann factor
    vector<double> pNeighbors;
    double pTotal = 0.0;
    for (vector<Agent *>::iterator i = m_neighbors.begin();
        i != m_neighbors.end(); i++)
    {
        double tmp = exp(((*i)->potential() - m_potential) * m_charge);
//		cout << "Calculating the probs (" << m_site << "): " << tmp << "\tS->T:\t" <<
//			m_potential << "->" << (*i)->potential();
        pNeighbors.push_back(tmp);
        pTotal += tmp;
//    cout << "\t Total: " << pTotal << endl;
    }
    // Now normalise and fit into the remaining probability
    double n = 1.0 - m_pBarrier;
    double previous = m_pBarrier;
    for (vector<double>::iterator i = pNeighbors.begin();
        i != pNeighbors.end(); i++)
    {
        *i = (*i / pTotal) * n + previous;
        previous = *i;
//		cout << *i << endl;
    }

//  cout << "Normalised probs: " << m_pBarrier << "\t";
//  for (vector<double>::iterator i = pNeighbors.begin();
//    i != pNeighbors.end(); i++)
//    cout << *i << "\t";
//  cout << "rn = " << rn << endl;
//	cout << "no of neighbors = " << m_neighbors.size()
//		<< " no of probs = " << pNeighbors.size() << endl;

    // Now to find which agent we should attempt transport to
    // The first case the lower limit is the m_pBarrier value.
    if (rn <= pNeighbors[0]) {
        if (m_neighbors[0]->acceptCharge(-1)) {
            m_fCharge = 0;
            return m_neighbors[0];
        }
        else
            return 0;
    }
    for (unsigned int i = 1; i < pNeighbors.size(); i++) {
        if (rn > pNeighbors[i-1] && rn <= pNeighbors[i]) {
            if (m_neighbors[i]->acceptCharge(-1)) {
                m_fCharge = 0;
                return m_neighbors[i];
            }
            else
        return 0;
        }
    }
    return 0;
}

void HoppingAgent::completeTick()
{
    // The future state becomes the current state.
    m_charge = m_fCharge;
}

} // End Langmuir namespace
