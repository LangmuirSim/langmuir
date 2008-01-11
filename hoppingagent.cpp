#include "hoppingagent.h"

#include "rand.h"

#include <iostream>

using namespace std;

namespace Langmuir
{

HoppingAgent::HoppingAgent(unsigned int site, const Grid* grid) :
	m_site(site), m_grid(grid), m_charge(0), m_fCharge(0),
	m_rand(new Rand(0., 1.)), m_pBarrier(0.5)
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
	return 0.;
}

bool HoppingAgent::acceptCharge(int charge)
{
	// Can only accept a charge if we are currently vacant and have not already
	// accepted a charge from another neighbour.
	if (m_charge == 0 && m_fCharge == 0)
	{
		m_fCharge = charge;
		return true;
	}
	else return false;
}

int HoppingAgent::charge()
{
	return m_charge;
}

Agent* HoppingAgent::transport()
{
	// Determine whether a transport event will be attempted this tick
	if (m_charge == 0)
		return 0;

	double rn = m_rand->number();
	if (rn <= m_pBarrier) return 0;
	
	// Select a random neighbor and attempt transport.
    int irn = int(m_rand->number() * double(m_neighbors.size()));
    if (m_neighbors[irn]->acceptCharge(-1)) 
    {
    	m_fCharge = 0;
    	return m_neighbors[irn];
    }
    else return 0;
	
	cout << "HoppingAgent " << m_site << " neighbours: " << m_neighbors.size() << endl;
	return 0;
}

void HoppingAgent::completeTick()
{
	// The future state becomes the current state.
	m_charge = m_fCharge;
}

} // End Langmuir namespace
