#include "openclhelper.h"
#include "chargeagent.h"
#include "drainagent.h"
#include "inputparser.h"
#include "simulation.h"
#include "potential.h"
#include "cubicgrid.h"
#include "world.h"
#include "rand.h"

namespace Langmuir
{
ChargeAgent::ChargeAgent(Agent::Type type, World * world, int site,QObject *parent): Agent(type, world, site, parent)
{
    m_site = m_fSite;
    m_charge = 0;
    m_removed = false;
    m_lifetime = 0;
    m_pathlength = 0;
    m_openClID = 0;
    m_grid = 0;
}

ElectronAgent::ElectronAgent(World *world, int site, QObject *parent): ChargeAgent(Agent::Electron, world, site, parent)
{
    m_grid = m_world->electronGrid();
    m_charge = -1;
    m_grid->registerAgent(this);
}

HoleAgent::HoleAgent(World *world, int site, QObject *parent): ChargeAgent(Agent::Hole, world, site, parent)
{
    m_grid = m_world->holeGrid();
    m_charge = 1;
    m_grid->registerAgent(this);
}

ChargeAgent::~ChargeAgent()
{
}

int ChargeAgent::charge()
{
    return m_charge;
}

bool ChargeAgent::removed()
{
    return m_removed;
}

int ChargeAgent::lifetime()
{
    return m_lifetime;
}

double ChargeAgent::pathlength()
{
    return m_pathlength;
}

void ChargeAgent::setOpenCLID(int id)
{
    m_openClID = id;
}

int ChargeAgent::getOpenCLID()
{
    return m_openClID;
}

void ChargeAgent::chooseFuture()
{
    // Select a proposed transport site at random
    m_fSite = m_neighbors[ m_world->randomNumberGenerator()->integer(0, m_neighbors.size()-1)];
}

void ChargeAgent::decideFuture()
{
    // Increase lifetime in existance
    m_lifetime += 1;

    switch(m_grid->agentType(m_fSite))
    {
    case Agent::Empty:
    {
        // Potential difference between sites
        double pd = m_grid->potential(m_fSite)- m_grid->potential(m_site);
        pd *= m_charge;

        // Coulomb interactions
        if(m_world->parameters()->coulombCarriers)
        {
            pd += this->coulombInteraction(m_fSite);
        }

        // Metropolis criterion
        if(m_world->randomNumberGenerator()->randomlyChooseYesWithMetropolisAndCoupling(
                 pd,
                 m_world->parameters()->inverseKT,
                 1.0/3.0))
        {
            // Accept move - increase distance traveled
            m_pathlength += 1;
            return;
        }
        else
        {
            // Reject move
            m_fSite = m_site;
        }
        return;
        break;
    }

    case Agent::Drain:
    {
        DrainAgent *drain = dynamic_cast<DrainAgent*>(m_grid->agentAddress(m_fSite));
        if(drain)
        {
            if(drain->tryToAccept(this))
            {
                m_pathlength += 1;
                break;
            }
        }
        // Shouldn't happen reject the move
        m_fSite = m_site;
        break;
    }

    default:
    {
        // Invalid site proposed(Defect, Electron, Hole, Source)
        m_fSite = m_site;
        break;
    }

    }
    return;
}

void ChargeAgent::completeTick()
{
    if(m_site != m_fSite)
    {   
        // If the future site is empty move along
        if(m_grid->agentType(m_fSite)== Agent::Empty)
        {            
            // Leave old site
            m_grid->unregisterAgent(this);

            // Enter new site
            m_site = m_fSite;
            m_grid->registerAgent(this);
            return;
        }

        // If the future site is the drain then remove charge
        if(m_grid->agentType(m_fSite)== Agent::Drain)
        {
            m_grid->unregisterAgent(this);
            m_removed = true;
            return;
        }

        // Abort the move - the site was not empty
        m_fSite = m_site;
        return;
    }
}

double ChargeAgent::coulombInteraction(int newSite)
{
    double p1 = 0;
    double p2 = 0;

    if(m_world->parameters()->useOpenCL)
    {
        // Assuming the GPU calculation output was copied to the CPU already
        p1 += m_world->opencl()->getOutputHost(m_openClID);
        p2 += m_world->opencl()->getOutputHostFuture(m_openClID);

        // Remove self interaction
        p2 -= m_world->interactionEnergies()[1][0][0] * m_charge;
    }
    else
    {
        // Electrons
        p1 += m_world->potential()->coulombPotentialElectrons(m_site);
        p2 += m_world->potential()->coulombPotentialElectrons(newSite);

        // Holes
        p1 += m_world->potential()->coulombPotentialHoles(m_site);
        p2 += m_world->potential()->coulombPotentialHoles(newSite);

        // Remove self interaction
        p2 -= m_world->interactionEnergies()[1][0][0] * m_charge;

        // Charged defects
        if(m_world->parameters()->coulombDefects)
        {
            p1 += m_world->potential()->coulombPotentialDefects(m_site);
            p2 += m_world->potential()->coulombPotentialDefects(newSite);
        }
    }

    //When holes and electrons on on the same site the interaction is not zero
    p2 += bindingPotential(newSite);

    return m_charge *(p2 - p1);
}

double HoleAgent::bindingPotential(int site)
{
    return 0.0;
}

double ElectronAgent::bindingPotential(int site)
{
    if(m_world->holeGrid()->agentType(site)== Agent::Hole)
    {
        return 0.5;
    }
    return 0.0;
}

void ChargeAgent::titleString( QTextStream &stream, SimulationParameters &par )
{
    stream.setRealNumberPrecision(par.outputPrecision );
    stream.setFieldWidth( par.outputWidth );
    stream.setRealNumberNotation( QTextStream::ScientificNotation );
    stream.setFieldAlignment( QTextStream::AlignRight );
    stream << "type"
           << "site_si"
           << "site_xi"
           << "site_yi"
           << "site_zi"
           << "site_sf"
           << "site_xf"
           << "site_yf"
           << "site_zf"
           << "lifetime"
           << "pathlength";
}

QTextStream& operator<<( QTextStream &stream, const ChargeAgent &charge )
{
    stream.setRealNumberPrecision( charge.m_world->parameters()->outputPrecision );
    stream.setFieldWidth( charge.m_world->parameters()->outputWidth );
    stream.setRealNumberNotation( QTextStream::ScientificNotation );
    stream.setFieldAlignment( QTextStream::AlignRight );
    stream << Agent::typeToQString(charge.m_type)
           << charge.m_site
           << charge.m_grid->getIndexX(charge.m_site)
           << charge.m_grid->getIndexY(charge.m_site)
           << charge.m_grid->getIndexZ(charge.m_site)
           << charge.m_fSite
           << charge.m_grid->getIndexX(charge.m_fSite)
           << charge.m_grid->getIndexY(charge.m_fSite)
           << charge.m_grid->getIndexZ(charge.m_fSite)
           << charge.m_lifetime
           << charge.m_pathlength;
    return stream;
}

}
