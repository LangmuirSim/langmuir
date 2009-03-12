#include "sourceagent.h"

#include "rand.h"

#include <iostream>

using namespace std;

namespace Langmuir
{

SourceAgent::SourceAgent(unsigned int site) : Agent(site),
    m_potential(0.), m_rand(new Rand(0.0, 1.0)), m_pBarrier(0.9), m_charges(0)
{
}

SourceAgent::SourceAgent(unsigned int site, double potential) : Agent(site),
    m_potential(potential), m_rand(new Rand(0., 1.0)), m_pBarrier(0.95)
{
}

SourceAgent::~SourceAgent()
{
    delete m_rand;
    m_rand = 0;
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

Agent* SourceAgent::transport()
{
    // Determine whether a transport event will be attempted this tick
    double rn = m_rand->number();
    if (rn <= m_pBarrier) return 0;

  // This is currently limiting the number of charges that can be injected
//	if (m_charges > 300) return 0;

    // Select a random neighbor and attempt transport.
    int irn = int(m_rand->number() * double(m_neighbors.size()));
    if (m_neighbors[irn]->acceptCharge(-1)) {
        m_charges++;
        return m_neighbors[irn];
    }
    else return 0;
}

} // End Langmuir namespace
