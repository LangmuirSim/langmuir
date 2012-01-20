#include "sourceagent.h"
#include "chargeagent.h"
#include "world.h"
#include "rand.h"

namespace Langmuir
{

SourceAgent::SourceAgent(World * world, double potential, double rate, int tries, QObject *parent)
    : FluxAgent(Agent::Source, world, potential, rate, tries, parent)
{
}

ElectronSourceAgent::ElectronSourceAgent(World * world, int site, double potential, double rate, int tries, QObject *parent)
    : SourceAgent(world, potential, rate, tries, parent)
{
    m_grid  = m_world->electronGrid();
    initializeSite(site);

    QString string;
    QTextStream stream(&string);
    stream << this->metaObject()->className() << "(" << this << ")";
    setObjectName(string);
}

ElectronSourceAgent::ElectronSourceAgent(World * world, Grid::CubeFace cubeFace, double potential, double rate, int tries, QObject *parent)
    : SourceAgent(world, potential, rate, tries, parent)
{
    m_grid  = m_world->electronGrid();
    initializeSite(cubeFace);

    QString string;
    QTextStream stream(&string);
    stream << this->metaObject()->className() << "(" << this << ")";
    setObjectName(string);
}

HoleSourceAgent::HoleSourceAgent(World * world, int site, double potential, double rate, int tries, QObject *parent)
    : SourceAgent(world, potential, rate, tries, parent)
{
    m_grid  = m_world->holeGrid();
    initializeSite(site);

    QString string;
    QTextStream stream(&string);
    stream << this->metaObject()->className() << "(" << this << ")";
    setObjectName(string);
}

HoleSourceAgent::HoleSourceAgent(World * world, Grid::CubeFace cubeFace, double potential, double rate, int tries, QObject *parent)
    : SourceAgent(world, potential, rate, tries, parent)
{
    m_grid  = m_world->holeGrid();
    initializeSite(cubeFace);

    QString string;
    QTextStream stream(&string);
    stream << this->metaObject()->className() << "(" << this << ")";
    setObjectName(string);
}

ExcitonSourceAgent::ExcitonSourceAgent(World * world, double rate, int tries, QObject *parent)
    : SourceAgent(world, 0.0, rate, tries, parent)
{
    m_grid = m_world->electronGrid();
    initializeSite(Grid::NoFace);
}

bool SourceAgent::seed()
{
    int site = randomSiteID();
    if(validToInject(site))
    {
        inject(site);
        return true;
    }
    return false;
}

bool SourceAgent::tryToInject()
{
    m_attempts += 1;
    int site = chooseSite();
    if(validToInject(site)&& shouldTransport(site))
    {
        inject(site);
        m_successes += 1;
        return true;
    }
    return false;
}

int SourceAgent::randomSiteID()
{
    return m_world->randomNumberGenerator()->integer(0, m_grid->volume()-1);
}

int SourceAgent::randomNeighborSiteID()
{
    return m_neighbors[m_world->randomNumberGenerator()->integer(0, m_neighbors.size()-1)];
}

int SourceAgent::chooseSite()
{
    return randomNeighborSiteID();
}

int ExcitonSourceAgent::chooseSite()
{
    return randomSiteID();
}

double ElectronSourceAgent::energyChange(int site)
{
    double p1 = m_potential;
    double p2 = m_grid->potential(site);
    return p1-p2; // its backwards because q=-1 and dE = q*(p2-p1)= p1-p2
}

double HoleSourceAgent::energyChange(int site)
{
    double p1 = m_potential;
    double p2 = m_grid->potential(site);
    return p2-p1;
}

double ExcitonSourceAgent::energyChange(int site)
{
    return 0;
}

void ElectronSourceAgent::inject(int site)
{
    ElectronAgent *electron = new ElectronAgent(m_world, site);
    m_world->electrons()->push_back(electron);
}

void HoleSourceAgent::inject(int site)
{
    HoleAgent *hole = new HoleAgent(m_world, site);
    m_world->holes()->push_back(hole);
}

void ExcitonSourceAgent::inject(int site)
{
    ElectronAgent *electron = new ElectronAgent(m_world, site);
    m_world->electrons()->push_back(electron);

    HoleAgent *hole = new HoleAgent(m_world, site);
    m_world->holes()->push_back(hole);
}

bool ElectronSourceAgent::validToInject(int site)
{
    if(electronCount()>= maxElectrons()||
         m_probability <= 0 ||
         m_tries <= 0 ||
         site < 0 ||
         site >= m_grid->volume()||
         m_grid->agentType(site)!= Agent::Empty ||
         m_grid->agentAddress(site)!= 0 ||
         m_world->defectSiteIDs()->contains(site))
    {
        return false;
    }
    return true;
}

bool HoleSourceAgent::validToInject(int site)
{
    if(holeCount()>= maxHoles()||
         m_probability <= 0 ||
         m_tries <= 0 ||
         site < 0 ||
         site >= m_grid->volume()||
         m_grid->agentType(site)!= Agent::Empty ||
         m_grid->agentAddress(site)!= 0 ||
         m_world->defectSiteIDs()->contains(site))
    {
        return false;
    }
    return true;
}

bool ExcitonSourceAgent::validToInject(int site)
{
    if(electronCount()>= maxElectrons()||
         holeCount()>= maxHoles()||
         m_probability <= 0 ||
         m_tries <= 0 ||
         site < 0 ||
         site >= m_world->electronGrid()->volume()||
         m_world->electronGrid()->agentType(site)!= Agent::Empty ||
         m_world->holeGrid()->agentType(site)!= Agent::Empty ||
         m_world->electronGrid()->agentAddress(site)!= 0 ||
         m_world->holeGrid()->agentAddress(site)!= 0 ||
         m_world->defectSiteIDs()->contains(site))
    {
        return false;
    }
    return true;
}
}
