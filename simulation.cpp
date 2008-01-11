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
//	cout << "Entered performIterations function.\n";
	Agent* t = 0;
	for (int i = 0; i < nIterations; i++)
	{
//		cout << i << endl;
		t = m_source->transport();
		if (t != 0)
		{
			// Successful current injection event
			m_fCharges.push_back(t);
//			cout << "Charge injected into site " << t->site() << "...\n";
		}
		for (unsigned int j = 0; j < m_charges.size(); j++)
		{
			t = m_charges[j]->transport();
			if (t == m_drain)
			{
//				cout << "Erased a charge that was accepted by the drain.\n";
				vector<Agent *>::iterator it = m_fCharges.begin() + j;
				m_fCharges.erase(it);
			}
			if (t != 0)
			{
//				cout << "Move: " << m_charges[j]->site() << " -> " << t->site() << endl;
				m_fCharges[j] = t;
			}
		}
		// Now we have finished this time tick
//		cout << "Time tick " << i << " completed, finalising state and moving on.\n";
		m_charges = m_fCharges;
		nextTick();
//		cout << "Charges at sites: ";
//		for (vector<Agent *>::iterator it = m_charges.begin();
//			it != m_charges.end(); it++)
//			cout << (*it)->site() << " ";
//		cout << endl;
		t = 0;
	}
}

void Simulation::nextTick()
{
	// Iterate over all sites to change their state
	for (vector<Agent *>::iterator it = m_agents.begin();
		it != m_agents.end(); it++)
		(*it)->completeTick();
}

void Simulation::printGrid()
{
	system("clear");
	unsigned int width = m_grid->getWidth();
	for (unsigned int i = 0; i < m_agents.size()-2; i++)
	{
		if (i % width == 0)
			cout << "||";
		if (m_agents[i]->charge() == -1)
			cout << "*";
		else
			cout << " ";
		if (i % width == width-1)
			cout << "||\n";
	}
}

void Simulation::createAgents(unsigned int num_agents, double sourcePotential,
		double drainPotential)
{
	m_agents.resize(num_agents+2);
	// Add the normal agents
	for (unsigned int i = 0; i < num_agents; i++)
	{
		m_agents[i] = new HoppingAgent(i, m_grid);
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
		cout << "Finding neighbours for site " << i << ": "; 
		for (unsigned int j = 0; j < neighbors.size(); j++)
		{
			cout << neighbors[j] << ", ";
			pNeighbors.push_back(m_agents[neighbors[j]]);
		}
		cout << endl;
		m_agents[i]->setNeighbors(pNeighbors);
		neighbors.clear();
		pNeighbors.clear();
	}
	updatePotentials();
}

void Simulation::destroyAgents()
{
	for (unsigned int i = 0; i < m_agents.size(); i++)
	{
		delete m_agents[i];
		m_agents[i] = 0;
	}
}

void Simulation::updatePotentials()
{
	// We are assuming one source, one drain that are parallel.
	// The most efficient way to calculate this is to work out the potential
	// for each column in our system and then assign it to each agent.
	int width = m_grid->getWidth();
	
	double m = (m_drain->potential() - m_source->potential()) / double(width);
	double c = m_source->potential();

	// The source to drain distance is simply the width of the device.
	for (int i = 0; i < width; i++)
	{
		double tPotential = m * (double(i)+0.5) + c;
		vector<unsigned int> neighbors = m_grid->col(i);
		for (vector<unsigned int>::iterator j = neighbors.begin(); 
			j != neighbors.end(); j++)
		{
			m_agents[*j]->setPotential(tPotential);
		}
		cout << "Row " << i << ", potential = " << tPotential << endl;
	}
}

} // End namespace Langmuir
