#include "openclhelper.h"
#include "chargeagent.h"
#include "drainagent.h"
#include "parameters.h"
#include "simulation.h"
#include "potential.h"
#include "cubicgrid.h"
#include "world.h"
#include "rand.h"

namespace Langmuir
{
ChargeAgent::ChargeAgent(Agent::Type type, World &world, Grid &grid, int site,QObject *parent)
    : Agent(type, world, site, parent), m_grid(grid)
{
    m_site = m_fSite;
    m_charge = 0;
    m_removed = false;
    m_lifetime = 0;
    m_pathlength = 0;
    m_openClID = 0;
}

ElectronAgent::ElectronAgent(World &world, int site, QObject *parent)
    : ChargeAgent(Agent::Electron, world, world.electronGrid(), site, parent)
{
    m_charge = -1;
    m_grid.registerAgent(this);
}

HoleAgent::HoleAgent(World &world, int site, QObject *parent)
    : ChargeAgent(Agent::Hole, world, world.holeGrid(), site, parent)
{
    m_charge = 1;
    m_grid.registerAgent(this);
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

int ChargeAgent::pathlength()
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
    m_fSite = m_neighbors[m_world.randomNumberGenerator().integer(0, m_neighbors.size()-1)];
}

Grid& ChargeAgent::getGrid()
{
    return m_grid;
}

void ChargeAgent::decideFuture()
{
    // Increase lifetime in existance
    m_lifetime += 1;

    switch(m_grid.agentType(m_fSite))
    {
    case Agent::Empty:
    {
        // Potential difference between sites
        double pd = m_grid.potential(m_fSite)- m_grid.potential(m_site);
        pd *= m_charge;

        // Coulomb interactions
        if(m_world.parameters().coulombCarriers)
        {
            pd += this->coulombInteraction(m_fSite);
        }

        // Calculate the coupling constant...
        int dx = m_grid.xDistancei(m_site,m_fSite);
        int dy = m_grid.yDistancei(m_site,m_fSite);
        int dz = m_grid.zDistancei(m_site,m_fSite);
        double coupling = m_world.coupling(dx,dy,dz);

        // Metropolis criterion
        if(m_world.randomNumberGenerator().randomlyChooseYesWithMetropolisAndCoupling(
                 pd,
                 m_world.parameters().inverseKT,
                 coupling))
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
        DrainAgent *drain = dynamic_cast<DrainAgent*>(m_grid.agentAddress(m_fSite));
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
        if(m_grid.agentType(m_fSite)== Agent::Empty)
        {            
            // Leave old site
            m_grid.unregisterAgent(this);

            // Enter new site
            m_site = m_fSite;
            m_grid.registerAgent(this);
            return;
        }

        // If the future site is the drain then remove charge
        if(m_grid.agentType(m_fSite)== Agent::Drain)
        {
            m_grid.unregisterAgent(this);
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

    if(m_world.parameters().useOpenCL)
    {
        // Assuming the GPU calculation output was copied to the CPU already
        p1 += m_world.opencl().getOutputHost(m_openClID);
        p2 += m_world.opencl().getOutputHostFuture(m_openClID);

        // Remove self interaction
        p2 -= m_world.interactionEnergies()[1][0][0] * m_charge;
    }
    else
    {
        // Electrons
        p1 += m_world.potential().coulombPotentialElectrons(m_site);
        p2 += m_world.potential().coulombPotentialElectrons(newSite);

        // Holes
        p1 += m_world.potential().coulombPotentialHoles(m_site);
        p2 += m_world.potential().coulombPotentialHoles(newSite);

        // Remove self interaction
        p2 -= m_world.interactionEnergies()[1][0][0] * m_charge;

        // Charged defects
        if(m_world.parameters().defectsCharge != 0)
        {
            p1 += m_world.potential().coulombPotentialDefects(m_site);
            p2 += m_world.potential().coulombPotentialDefects(newSite);
        }
    }

    //When holes and electrons on on the same site the interaction is not zero
    p2 += bindingPotential(newSite);
    p1 += bindingPotential(m_site);

    return m_charge *(p2 - p1);
}

double HoleAgent::bindingPotential(int site)
{
    if(m_world.holeGrid().agentType(site)== Agent::Electron)
    {
        return -0.5;
    }
    return 0.0;
}

double ElectronAgent::bindingPotential(int site)
{
    if(m_world.holeGrid().agentType(site)== Agent::Hole)
    {
        return 0.5;
    }
    return 0.0;
}

}
