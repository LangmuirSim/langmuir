#include "tolerance.h"
#include "fluxagent.h"
#include "world.h"
#include "parameters.h"

namespace Langmuir
{

Tolerance::Tolerance(World &world, FluxAgent *flux, double criteria, QObject *parent) :
    QObject(parent),
    m_current(0.0),
    m_previous(0.0),
    m_criteria(criteria),
    m_converged(0),
    m_flux(flux),
    m_world(world)
{
    checkCritera();
    setPrevious();
    setCurrent();
}

void Tolerance::checkCritera()
{
    if (m_criteria < 0 || m_criteria > 1)
    {
        qFatal("invalid criteria for convergence: %f",m_criteria);
    }
}

void Tolerance::setPrevious()
{
    m_previous = m_flux->successRate();
}

void Tolerance::setCurrent()
{
    m_current  = m_flux->successRate();
}

double Tolerance::percentChange()
{
    if(m_previous > 0)
    {
        return abs(m_current - m_previous)/double(m_previous);
    }
    else
    {
        return 0.0;
    }
}

void Tolerance::check()
{
    setCurrent();
    if (percentChange() <= m_criteria)
    {
        m_converged++;
    }
    else
    {
        m_converged = 0;
    }
    setPrevious();
}

bool Tolerance::converged()
{
    if (m_converged >= 10)
    {
        return true;
    }
    return false;
}

}
