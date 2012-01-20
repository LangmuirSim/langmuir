#include "drainagent.h"
#include "chargeagent.h"
#include "world.h"
#include "rand.h"

namespace Langmuir
{

DrainAgent::DrainAgent(World * world, double potential, double rate, int tries, QObject *parent)
    : FluxAgent(Agent::Drain, world, potential, rate, tries, parent)
{
}

ElectronDrainAgent::ElectronDrainAgent(World * world, int site, double potential, double rate, int tries, QObject *parent)
    : DrainAgent(world, potential, rate, tries, parent)
{
    m_grid  = m_world->electronGrid();
    initializeSite(site);

    QString string;
    QTextStream stream(&string);
    stream << this->metaObject()->className() << "(" << this << ")";
    setObjectName(string);
}

ElectronDrainAgent::ElectronDrainAgent(World * world, Grid::CubeFace cubeFace, double potential, double rate, int tries, QObject *parent)
    : DrainAgent(world, potential, rate, tries, parent)
{
    m_grid  = m_world->electronGrid();
    initializeSite(cubeFace);

    QString string;
    QTextStream stream(&string);
    stream << this->metaObject()->className() << "(" << this << ")";
    setObjectName(string);
}

HoleDrainAgent::HoleDrainAgent(World * world, int site, double potential, double rate, int tries, QObject *parent)
    : DrainAgent(world, potential, rate, tries, parent)
{
    m_grid  = m_world->holeGrid();
    initializeSite(site);

    QString string;
    QTextStream stream(&string);
    stream << this->metaObject()->className() << "(" << this << ")";
    setObjectName(string);
}

HoleDrainAgent::HoleDrainAgent(World * world, Grid::CubeFace cubeFace, double potential, double rate, int tries, QObject *parent)
    : DrainAgent(world, potential, rate, tries, parent)
{
    m_grid  = m_world->holeGrid();
    initializeSite(cubeFace);

    QString string;
    QTextStream stream(&string);
    stream << this->metaObject()->className() << "(" << this << ")";
    setObjectName(string);
}

bool DrainAgent::tryToAccept(ChargeAgent *charge)
{
    m_attempts += 1;
    if(shouldTransport(charge->site()) )
    {
        m_successes += 1;
        return true;
    }
    return false;
}

double ElectronDrainAgent::energyChange(int site)
{
    double p1 = m_potential;
    double p2 = m_grid->potential(site);
    return p1-p2; // its backwards because q=-1 and dE = q*(p2-p1)= p1-p2
}

double HoleDrainAgent::energyChange(int site)
{
    double p1 = m_potential;
    double p2 = m_grid->potential(site);
    return p2-p1;
}

}
