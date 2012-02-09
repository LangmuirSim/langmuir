#include "drainagent.h"
#include "chargeagent.h"
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

bool DrainAgent::tryToAccept(ChargeAgent *charge)
{
    m_attempts += 1;
    if(shouldTransport(charge->getCurrentSite()) )
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

}
