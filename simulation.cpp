#include "simulation.h"

#include "cubicgrid.h"
#include "hoppingagent.h"
#include "sourceagent.h"
#include "drainagent.h"

using namespace std;

namespace Langmuir
{

Simulation::Simulation(unsigned int width, unsigned int height, double sourcePotential,
		double drainPotential)
{
	m_grid = new CubicGrid(width, height);

	// Create the agents for the simulation
	createAgents(width * height, sourcePotential, drainPotential);
}

Simulation::~Simulation()
{
	delete m_grid;
	m_grid = 0;
	destroyAgents();
}

void Simulation::createAgents(unsigned int num_agents, double sourcePotential,
		double drainPotential)
{
	m_agents.resize(num_agents+2);
	// Add the normal agents
	for (unsigned int i = 0; i < num_agents; i++)
	{
		m_agents[i] = new HoppingAgent;
	}
	// Add the source and the drain
	m_source = new SourceAgent(sourcePotential);
	m_agents[num_agents] = m_source;
	m_drain = new DrainAgent(drainPotential);
	m_agents[num_agents+1] = m_drain;

	for (unsigned int i = 0; i < num_agents; i++)
	{
		vector<unsigned int> neighbors = m_grid->getNeighbors(i);
		vector<Agent *> pNeighbors;
		for (unsigned int j = 0; j < neighbors.size(); j++)
		{
			pNeighbors.push_back(m_agents[neighbors[j]]);
		}
		m_agents[i]->setNeighbors(pNeighbors);
	}
}

void Simulation::destroyAgents()
{
	for (unsigned int i = 0; i < m_agents.size(); i++)
	{
		delete m_agents[i];
		m_agents[i] = 0;
	}
}

} // End namespace Langmuir
