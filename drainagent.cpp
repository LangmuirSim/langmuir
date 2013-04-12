#include "drainagent.h"
#include "chargeagent.h"
#include "parameters.h"
#include "writer.h"
#include "world.h"
#include "rand.h"

namespace Langmuir
{

DrainAgent::DrainAgent(World &world, Grid &grid, QObject *parent)
    : FluxAgent(Agent::Drain, world, grid, parent)
{
}

ElectronDrainAgent::ElectronDrainAgent(World &world, int site, QObject *parent)
    : DrainAgent(world, world.electronGrid(), parent)
{
    initializeSite(site);
    QString name;
    QTextStream stream(&name);
    stream << "e" << m_type << site;
    stream.flush();
    setObjectName(name);
}

ElectronDrainAgent::ElectronDrainAgent(World &world, Grid::CubeFace cubeFace, QObject *parent)
    : DrainAgent(world, world.electronGrid(), parent)
{
    initializeSite(cubeFace);
    QString name;
    QTextStream stream(&name);
    stream << "e" << m_type << faceToLetter();
    stream.flush();
    setObjectName(name);
}

HoleDrainAgent::HoleDrainAgent(World &world, int site, QObject *parent)
    : DrainAgent(world, world.holeGrid(), parent)
{
    initializeSite(site);
    QString name;
    QTextStream stream(&name);
    stream << "h" << m_type << site;
    stream.flush();
    setObjectName(name);
}

HoleDrainAgent::HoleDrainAgent(World &world, Grid::CubeFace cubeFace, QObject *parent)
    : DrainAgent(world, world.holeGrid(), parent)
{
    initializeSite(cubeFace);
    QString name;
    QTextStream stream(&name);
    stream << "h" << m_type << faceToLetter();
    stream.flush();
    setObjectName(name);
}

RecombinationAgent::RecombinationAgent(World &world, QObject *parent)
    : DrainAgent(world, world.electronGrid(), parent)
{
    initializeSite(Grid::NoFace);
    QString name;
    QTextStream stream(&name);
    stream << "x" << m_type;
    stream.flush();
    setObjectName(name);
}

void RecombinationAgent::guessProbability()
{
    if ((m_attempts > 0) && (m_world.parameters().currentStep > 0))
    {
        m_probability = (m_world.parameters().recombinationRate * m_world.parameters().currentStep) / m_attempts;
    }
    else
    {
        m_probability = m_world.parameters().recombinationRate;
    }
}

bool DrainAgent::tryToAccept(ChargeAgent *charge)
{
    m_attempts += 1;
    if(shouldTransport(charge->getCurrentSite()))
    {
        m_successes += 1;
        return true;
    }
    return false;
}

double ElectronDrainAgent::energyChange(int site)
{
    double p1 = m_potential;
    double p2 = m_grid.potential(site);
    return p1-p2; // its backwards because q=-1 and dE = q*(p2-p1)= p1-p2
}

double HoleDrainAgent::energyChange(int site)
{
    double p1 = m_potential;
    double p2 = m_grid.potential(site);
    return p2-p1;
}

double RecombinationAgent::energyChange(int site)
{
    return 0.0;
}

bool RecombinationAgent::tryToAccept(ChargeAgent *charge)
{
    // The current site
    int site = charge->getCurrentSite();

    // A list of neighboring sites WITH carriers
    QVector<int> neighbors;

    // Consider same-site neighbors
    if (charge->otherGrid().agentType(site) == charge->otherType())
    {
        neighbors.push_back(charge->getCurrentSite());
    }

    // Add more possibilities
    switch (m_world.parameters().recombinationRange)
    {
        // Only consider same-site neighbors, which we did above
        case 0:
        {
            break;
        }

        // Consider more neighbors
        default:
        {
            // Don't construct a neighbor list if one already exists
            if (m_world.parameters().hoppingRange == m_world.parameters().recombinationRange)
            {
                // Loop over the neighbors and check if charges are there
                foreach (int otherSite, charge->getNeighbors())
                {
                    if (charge->otherGrid().agentType(otherSite) == charge->otherType())
                    {
                        neighbors.push_back(otherSite);
                    }
                }
            }
            else
            {
                // We need to make a new neighborlist
                QVector<int> constructed_neighbors = m_grid.neighborsSite(
                            charge->getCurrentSite(),
                            m_world.parameters().recombinationRange);

                // Loop over the neighbors and check if charges are there
                foreach (int otherSite, constructed_neighbors)
                {
                    if (charge->otherGrid().agentType(otherSite) == charge->otherType())
                    {
                        neighbors.push_back(otherSite);
                    }
                }
            }
            break;
        }
    }

    // Found charges to recombine with
    if (neighbors.size() > 0)
    {
        // Randomly choose which neighboring site to recombine with
        int recombiningSite = neighbors[m_world.randomNumberGenerator().integer(
                    0, neighbors.size()-1)];

        // Get the other ChargeAgent
        ChargeAgent *other = dynamic_cast<ChargeAgent*>(
                    charge->otherGrid().agentAddress(recombiningSite));

        // Make sure things are OK with the other ChargeAgent
        if (!other)
        {
             qFatal("message: dynamic cast from Agent* to ChargeAgent* has failed during recombination");
        }

        // Recombination already happened
        if (other->removed())
        {
            return false;
        }

        // Try to recombine
        m_attempts += 1;

        if(shouldTransport(recombiningSite))
        {
            // RecombinationAgent has succeeded
            m_successes += 1;

            // Remove both charges
            charge->setRemoved(true);
            other->setRemoved(true);

            // A recombination occured
            return true;
        }
        // No recombination occured
        return false;
    }
    // No charges to recombine with found
    return false;
}

}
