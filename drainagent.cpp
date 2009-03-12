#include "drainagent.h"

using namespace std;

namespace Langmuir
{

DrainAgent::DrainAgent(World *world, unsigned int site) : Agent(world, site),
    m_potential(0.0)
{
}

DrainAgent::DrainAgent(World *world, unsigned int site, double potential) :
    Agent(world, site), m_potential(potential)
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
    return true;
}

int DrainAgent::charge()
{
    // Always has a charge of 1
    return 1;
}

Agent* DrainAgent::transport()
{
    return 0;
}

} // End Langmuir namespace
