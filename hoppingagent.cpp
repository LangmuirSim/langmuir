#include "hoppingagent.h"

using namespace std;

namespace Langmuir
{

HoppingAgent::HoppingAgent(unsigned int site) :
	m_site(site), m_charge(0), m_futureCharge(0)
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
	// Can only accept a charge if we are currently vacant and have not already
	// accepted a charge from another neighbour.
	if (m_charge == 0 && m_futureCharge == 0)
	{
		m_futureCharge = charge;
		return true;
	}
	else return false;
}

int HoppingAgent::charge()
{
	return m_charge;
}

unsigned int HoppingAgent::transport()
{
	return m_site;
}

void HoppingAgent::completeTick()
{
	// The future state becomes the current state.
	m_charge = m_futureCharge;
}

} // End Langmuir namespace
