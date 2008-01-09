#include "drainagent.h"

using namespace std;

namespace Langmuir
{

DrainAgent::DrainAgent(unsigned int site) :
	m_site(site), m_potential(0.)
{
}

DrainAgent::DrainAgent(unsigned int site, double potential) :
	m_site(site), m_potential(potential)
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

unsigned int DrainAgent::transport()
{
	return m_site;
}

} // End Langmuir namespace
