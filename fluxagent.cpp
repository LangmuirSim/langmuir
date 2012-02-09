#include "fluxagent.h"
#include "parameters.h"
#include "world.h"
#include "rand.h"

namespace Langmuir
{

FluxAgent::FluxAgent(Agent::Type type, World &world, Grid &grid, QObject *parent)
    : Agent(type, world,-1,parent), m_grid(grid)
{
    m_attempts      = 0;
    m_successes     = 0;
    m_potential     = 0;
    m_probability   = 0;
    m_tries         = 1;
    m_face          = Grid::NoFace;
}

void FluxAgent::initializeSite(int site)
{
    m_site = site;
    m_fSite = site;
    m_grid.registerAgent(this);
}

void FluxAgent::initializeSite(Grid::CubeFace cubeFace)
{
    m_grid.registerSpecialAgent(this, cubeFace);
    m_face = cubeFace;
}

FluxAgent::~FluxAgent()
{
    m_grid.unregisterSpecialAgent(this, m_face);
}

unsigned long int FluxAgent::attempts()const
{
    return m_attempts;
}

unsigned long int FluxAgent::successes()const
{
    return m_successes;
}

double FluxAgent::successRate()const
{
    if(m_attempts > 0)
    {
        return double(m_successes)/double(m_attempts)*100.0;
    }
    else
    {
        return 0.0;
    }
}

void FluxAgent::resetCounters()
{
    m_attempts = 0;
    m_successes = 0;
}

bool FluxAgent::shouldTransport(int site)
{
    return m_world.randomNumberGenerator().randomlyChooseYesWithPercent(m_probability);
    //return m_world.randomNumberGenerator().randomlyChooseYesWithMetropolisAndCoupling(
    //        energyChange(site), m_world.parameters().inverseKT, m_rate);
}

double FluxAgent::energyChange(int site)
{
    return 0;
}

Grid::CubeFace FluxAgent::face() const
{
    return m_face;
}

void FluxAgent::setPotential(double potential)
{
    m_potential = potential;
}

void FluxAgent::setRate(double rate)
{
    m_probability = rate;
}

void FluxAgent::setTries(int tries)
{
    m_tries = tries;
}

QString FluxAgent::faceToLetter()
{
    QString string;
    QTextStream stream(&string);
    stream << m_face;
    stream.flush();
    return string.at(0);
}

}
