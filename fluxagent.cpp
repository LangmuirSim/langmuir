#include "fluxagent.h"
#include "chargeagent.h"
#include "potential.h"
#include "inputparser.h"
#include "logger.h"
#include "world.h"
#include "rand.h"
#include "limits.h"

namespace Langmuir
{

int FluxAgent::m_maxHoles     = 0;
int FluxAgent::m_maxElectrons = 0;

FluxAgent::FluxAgent(Agent::Type type, World * world, double potential, double rate, int tries, QObject *parent): Agent(type, world,-1,parent)
{
    m_attempts     = 0;
    m_successes    = 0;
    m_grid         = 0;
    m_potential    = potential;
    m_probability         = rate;
    m_tries        = tries;
    m_face         = Grid::NoFace;
    m_maxHoles     = m_world->parameters()->holePercentage*double(m_world->holeGrid()->volume());
    m_maxElectrons = m_world->parameters()->electronPercentage*double(m_world->electronGrid()->volume());
}

void FluxAgent::initializeSite(int site)
{
    m_site = site;
    m_fSite = site;
    m_grid->registerAgent(this);
}

void FluxAgent::initializeSite(Grid::CubeFace cubeFace)
{
    m_grid->registerSpecialAgent(this, cubeFace);
    m_face = cubeFace;
}

FluxAgent::~FluxAgent()
{
    if(m_grid)
    {
        m_grid->unregisterSpecialAgent(this, m_face);
    }
}

int FluxAgent::maxCarriers()const
{
    return maxElectrons()+ maxHoles();
}

int FluxAgent::maxElectrons()const
{
    return m_maxElectrons;
}

int FluxAgent::maxHoles()const
{
    return m_maxHoles;
}

int FluxAgent::carrierCount()const
{
    return m_world->electrons()->size()+ m_world->holes()->size();
}

int FluxAgent::electronCount()const
{
    return m_world->electrons()->size();
}

int FluxAgent::holeCount()const
{
    return m_world->holes()->size();
}

double FluxAgent::percentCarriers()const
{
    if(maxCarriers()> 0)
    {
        return double(carrierCount())/double(maxCarriers())*100.0;
    }
    else
    {
        return 0.0;
    }
}

double FluxAgent::percentHoles()const
{
    if(maxHoles()> 0)
    {
        return double(holeCount())/double(maxHoles())*100.0;
    }
    else
    {
        return 0.0;
    }
}

double FluxAgent::percentElectrons()const
{
    if(maxElectrons()> 0)
    {
        return double(electronCount())/double(maxElectrons())*100.0;
    }
    else
    {
        return 0.0;
    }
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
    return m_world->randomNumberGenerator()->randomlyChooseYesWithPercent(m_probability);
    //return m_world->randomNumberGenerator()->randomlyChooseYesWithMetropolisAndCoupling(
    //        energyChange(site), m_world->parameters()->inverseKT, m_rate);
}

double FluxAgent::energyChange(int site)
{
    return 0;
}

void FluxAgent::titleString( QTextStream &stream, SimulationParameters &par )
{
    stream.setRealNumberPrecision(par.outputPrecision );
    stream.setFieldWidth( par.outputWidth );
    stream.setRealNumberNotation( QTextStream::ScientificNotation );
    stream.setFieldAlignment( QTextStream::AlignRight );
    stream << "type"
           << "face"
           << "site"
           << "attempts"
           << "successes"
           << "rate"
           << "hMax"
           << "hCount"
           << "eMax"
           << "eCount"
           << "probability"
           << "potential"
           << "tries";
}

QTextStream& operator<<( QTextStream &stream, const FluxAgent &flux )
{
    stream.setRealNumberPrecision( flux.m_world->parameters()->outputPrecision );
    stream.setFieldWidth( flux.m_world->parameters()->outputWidth );
    stream.setRealNumberNotation( QTextStream::ScientificNotation );
    stream.setFieldAlignment( QTextStream::AlignRight );
    stream << Agent::typeToQString(flux.m_type)
           << flux.m_grid->cubeFaceToQString(flux.m_face)
           << flux.m_site
           << flux.attempts()
           << flux.successes()
           << flux.successRate()
           << flux.maxHoles()
           << flux.holeCount()
           << flux.maxElectrons()
           << flux.electronCount()
           << flux.m_probability
           << flux.m_potential
           << flux.m_tries;
    return stream;
}

}
