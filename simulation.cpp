#include "simulation.h"

#include "cubicgrid.h"
#include "hoppingagent.h"
#include "sourceagent.h"
#include "drainagent.h"

#include <iostream>

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

void Simulation::performIterations(int nIterations)
{
	cout << "Entered performIterations function.\n";
	unsigned int t = m_agents.size() + 2;
	cout << "Starting main loop.\n";
	for (int i = 0; i < nIterations; i++)
	{
		cout << i << endl;
		t = m_source->transport();
		if (t != m_agents.size() - 2)
		{
			// Successful current injection event
			cout << "Charge injected into site " << t << "...\n";
		}
	}
}

void Simulation::createAgents(unsigned int num_agents, double sourcePotential,
		double drainPotential)
{
	m_agents.resize(num_agents+2);
	// Add the normal agents
	for (unsigned int i = 0; i < num_agents; i++)
	{
		m_agents[i] = new HoppingAgent(i);
	}
	// Add the source and the drain
	m_source = new SourceAgent(num_agents, sourcePotential);
	m_agents[num_agents] = m_source;
	m_drain = new DrainAgent(num_agents+1, drainPotential);
	m_agents[num_agents+1] = m_drain;
	// Now to assign nearest neighbours for the electrodes.
	vector<unsigned int> neighbors = m_grid->col(0);
	vector<Agent *> pNeighbors;
	for (unsigned int j = 0; j < neighbors.size(); j++)
	{
		pNeighbors.push_back(m_agents[neighbors[j]]);
	}
	m_source->setNeighbors(pNeighbors);
	neighbors.clear();
	pNeighbors.clear();
	
	neighbors = m_grid->col(m_grid->getWidth()-1);
	for (unsigned int j = 0; j < neighbors.size(); j++)
	{
		pNeighbors.push_back(m_agents[neighbors[j]]);
	}
	m_drain->setNeighbors(pNeighbors);
	neighbors.clear();
	pNeighbors.clear();

	for (unsigned int i = 0; i < num_agents; i++)
	{
		neighbors = m_grid->getNeighbors(i);
		for (unsigned int j = 0; j < neighbors.size(); j++)
		{
			pNeighbors.push_back(m_agents[neighbors[j]]);
		}
		m_agents[i]->setNeighbors(pNeighbors);
		neighbors.clear();
		pNeighbors.clear();
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
