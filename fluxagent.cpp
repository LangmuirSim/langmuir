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
    m_face          = Grid::NoFace;
    storeLast();
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

void FluxAgent::storeLast()
{
    m_lastSuccesses = m_successes;
    m_lastAttempts = m_attempts;
    m_lastStep = m_world.parameters().currentStep;
}

unsigned long int FluxAgent::successesSinceLast() const
{
    return m_successes - m_lastSuccesses;
}

unsigned long int FluxAgent::attemptsSinceLast() const
{
    return m_attempts - m_lastAttempts;
}

unsigned long int FluxAgent::stepsSinceLast() const
{
    return m_world.parameters().currentStep - m_lastStep;
}

double FluxAgent::successProbability()const
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

double FluxAgent::successRate()const
{
    if(m_world.parameters().currentStep > 0)
    {
        return double(m_successes)/double(m_world.parameters().currentStep);
    }
    else
    {
        return 0.0;
    }
}

double FluxAgent::successProbabilitySinceLast() const
{
    if(attemptsSinceLast() > 0)
    {
        return double(successesSinceLast())/double(attemptsSinceLast())*100.0;
    }
    else
    {
        return 0.0;
    }
}

double FluxAgent::successRateSinceLast() const
{
    if(stepsSinceLast() > 0)
    {
        return double(successesSinceLast())/double(stepsSinceLast());
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
    storeLast();
}

bool FluxAgent::shouldTransport(int site)
{
    return m_world.randomNumberGenerator().chooseYes(m_probability);
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

QString FluxAgent::faceToLetter()
{
    QString string;
    QTextStream stream(&string);
    stream << m_face;
    stream.flush();
    return string.at(0);
}

void FluxAgent::setAttempts(unsigned long int value)
{
    m_attempts = value;
    storeLast();
}

void FluxAgent::setSuccesses(unsigned long int value)
{
    m_successes = value;
    storeLast();
}

double FluxAgent::potential()const
{
    return m_potential;
}

double FluxAgent::rate()const
{
    return m_probability;
}

Grid& FluxAgent::grid()const
{
    return m_grid;
}

}
