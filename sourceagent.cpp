#include "sourceagent.h"
#include "chargeagent.h"
#include "world.h"
#include "rand.h"

namespace Langmuir
{

SourceAgent::SourceAgent(World &world, Grid& grid, QObject *parent)
    : FluxAgent(Agent::Source, world, grid, parent)
{
}

ElectronSourceAgent::ElectronSourceAgent(World &world, int site, QObject *parent)
    : SourceAgent(world, world.electronGrid(), parent)
{
    initializeSite(site);
    QString name;
    QTextStream stream(&name);
    stream << "e" << m_type << site;
    stream.flush();
    setObjectName(name);
}

ElectronSourceAgent::ElectronSourceAgent(World &world, Grid::CubeFace cubeFace, QObject *parent)
    : SourceAgent(world, world.electronGrid(), parent)
{
    initializeSite(cubeFace);
    QString name;
    QTextStream stream(&name);
    stream << "e" << m_type << faceToLetter();
    stream.flush();
    setObjectName(name);
}

HoleSourceAgent::HoleSourceAgent(World &world, int site, QObject *parent)
    : SourceAgent(world, world.holeGrid(), parent)
{
    initializeSite(site);
    QString name;
    QTextStream stream(&name);
    stream << "h" << m_type << site;
    stream.flush();
    setObjectName(name);
}

HoleSourceAgent::HoleSourceAgent(World &world, Grid::CubeFace cubeFace, QObject *parent)
    : SourceAgent(world, world.holeGrid(), parent)
{
    initializeSite(cubeFace);
    QString name;
    QTextStream stream(&name);
    stream << "h" << m_type << faceToLetter();
    stream.flush();
    setObjectName(name);
}

ExcitonSourceAgent::ExcitonSourceAgent(World &world, QObject *parent)
    : SourceAgent(world, world.electronGrid(), parent)
{
    initializeSite(Grid::NoFace);
    QString name;
    QTextStream stream(&name);
    stream << "x" << m_type;
    stream.flush();
    setObjectName(name);
}

bool SourceAgent::tryToSeed()
{
    int site = randomSiteID();
    if(validToInject(site))
    {
        inject(site);
        return true;
    }
    return false;
}

bool SourceAgent::tryToSeed(int site)
{
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
    return m_world.randomNumberGenerator().integer(0, m_grid.volume()-1);
}

int SourceAgent::randomNeighborSiteID()
{
    return m_neighbors[m_world.randomNumberGenerator().integer(0, m_neighbors.size()-1)];
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
    double p2 = m_grid.potential(site);
    return p1-p2; // its backwards because q=-1 and dE = q*(p2-p1)= p1-p2
}

double HoleSourceAgent::energyChange(int site)
{
    double p1 = m_potential;
    double p2 = m_grid.potential(site);
    return p2-p1;
}

double ExcitonSourceAgent::energyChange(int site)
{
    return 0;
}

void ElectronSourceAgent::inject(int site)
{
    ElectronAgent *electron = new ElectronAgent(m_world, site);
    m_world.electrons().push_back(electron);
}

void HoleSourceAgent::inject(int site)
{
    HoleAgent *hole = new HoleAgent(m_world, site);
    m_world.holes().push_back(hole);
}

void ExcitonSourceAgent::inject(int site)
{
    ElectronAgent *electron = new ElectronAgent(m_world, site);
    m_world.electrons().push_back(electron);

    HoleAgent *hole = new HoleAgent(m_world, site);
    m_world.holes().push_back(hole);
}

bool ElectronSourceAgent::validToInject(int site)
{
    if( m_world.numElectronAgents() >= m_world.maxElectronAgents() ||
        m_probability <= 0 ||
        m_tries <= 0 ||
        site < 0 ||
        site >= m_grid.volume()||
        m_grid.agentType(site)!= Agent::Empty ||
        m_grid.agentAddress(site)!= 0 ||
        m_world.defectSiteIDs().contains(site))
    {
        return false;
    }
    return true;
}

bool HoleSourceAgent::validToInject(int site)
{
    if( m_world.numHoleAgents() >= m_world.maxHoleAgents() ||
        m_probability <= 0 ||
        m_tries <= 0 ||
        site < 0 ||
        site >= m_grid.volume()||
        m_grid.agentType(site)!= Agent::Empty ||
        m_grid.agentAddress(site)!= 0 ||
        m_world.defectSiteIDs().contains(site))
    {
        return false;
    }
    return true;
}

bool ExcitonSourceAgent::validToInject(int site)
{
    if( m_world.numElectronAgents() >= m_world.maxElectronAgents() ||
        m_world.numHoleAgents() >= m_world.maxHoleAgents() ||
        m_probability <= 0 ||
        m_tries <= 0 ||
        site < 0 ||
        site >= m_world.electronGrid().volume()||
        m_world.electronGrid().agentType(site)!= Agent::Empty ||
        m_world.holeGrid().agentType(site)!= Agent::Empty ||
        m_world.electronGrid().agentAddress(site)!= 0 ||
        m_world.holeGrid().agentAddress(site)!= 0 ||
        m_world.defectSiteIDs().contains(site))
    {
        return false;
    }
    return true;
}
}
