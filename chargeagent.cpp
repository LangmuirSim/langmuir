#include "openclhelper.h"
#include "chargeagent.h"
#include "drainagent.h"
#include "inputparser.h"
#include "simulation.h"
#include "potential.h"
#include "world.h"
#include "grid.h"
#include "rand.h"


namespace Langmuir
{
    ChargeAgent::ChargeAgent (Agent::Type type, World * world, int site):Agent (type, world, site),
     m_charge (0), m_removed (false), m_lifetime(0), m_distanceTraveled(0.0), m_openClID(0)
  {
    //current site and future site are the same
    m_site = m_fSite;

    switch ( m_type )
    {
        case Agent::Electron:
        {
            m_grid = m_world->electronGrid();
            m_charge = -1;
            break;
        }
        case Agent::Hole:
        {
            m_grid = m_world->holeGrid();
            m_charge = +1;
            break;
        }
        default:
        {
            qFatal("ChargeAgent has invalid Agent:Type: %s",
                   qPrintable(Agent::typeToQString(m_grid->agentType(m_fSite))));
            break;
        }
    }

    if ( m_grid->agentAddress(m_site) == 0 && m_grid->agentType(m_site) == Agent::Empty )
    {
        //make sure this agent is somewhere on the grid
        m_grid->setAgentAddress(m_site,this);
        m_grid->setAgentType(m_site,m_type);
    }
    else
    {
        qFatal("chargeAgent can not be placed at site, site is already occupied");
    }
    //figure out the neighbors of this site
    m_neighbors = m_grid->neighborsSite(m_site);
  }

  ChargeAgent::~ChargeAgent ()
  {
  }

  int ChargeAgent::charge()
  {
    return m_charge;
  }

  void ChargeAgent::setCharge( int value )
  {
    m_charge = value;
  }

  bool ChargeAgent::removed()
  {
    return m_removed;
  }

  int ChargeAgent::lifetime()
  {
   return m_lifetime;
  }

  double ChargeAgent::distanceTraveled()
  {
   return m_distanceTraveled;
  }

  void ChargeAgent::setOpenCLID( int id )
  {
      m_openClID = id;
  }

  int ChargeAgent::getOpenCLID( )
  {
      return m_openClID;
  }

  void ChargeAgent::chooseFuture()
  {
    // Select a proposed transport site at random
    m_fSite = m_neighbors[ m_world->randomNumberGenerator()->integer(0,m_neighbors.size()-1) ];
  }

  void ChargeAgent::decideFuture()
  {
    // Increase lifetime in existance
    m_lifetime += 1;

    switch ( m_grid->agentType(m_fSite) )
    {
        case Agent::Empty:
        {
            // Potential difference between sites
            double pd = m_grid->potential(m_fSite) - m_grid->potential (m_site);
            pd *= m_charge;

            // Coulomb interactions
            if ( m_world->parameters()->interactionCoulomb )
            {
                pd += this->coulombInteraction (m_fSite);
            }

            // Metropolis criterion
            if ( m_world->randomNumberGenerator()->randomlyChooseYesWithMetropolisAndCoupling(
                        pd,
                        m_world->parameters()->inverseKT,
                        m_world->coupling()[m_grid->agentType(m_site)][m_grid->agentType(m_fSite)]) )
            {
                // Accept move - increase distance traveled
                m_distanceTraveled += 1.0;
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

        case Agent::HoleDrainL: case Agent::HoleDrainR:
        {
            // Move to drain with a constant probability ( see coupling constants )
            if ( m_world->holeDrainL()->tryToAccept(this) )
            {
                // Accept move - increase distance traveled
                m_distanceTraveled += 1.0;
            }
            else
            {
                // Reject move
                m_fSite = m_site;
            }
            break;
        }

        case Agent::Electron: case Agent::Hole: case Agent::Defect:
        {
            // Reject move - do not move to where there are Electrons, Holes, or Defects
            m_fSite = m_site;
            break;
        }

        case Agent::HoleSourceL: case Agent::HoleSourceR: case Agent::ElectronSourceL: case Agent::ElectronSourceR:
        {
            // Kill program - this should never happen
            qFatal("ChargeAgent::decideFuture transport to a Source agent: %s",
                   qPrintable(Agent::typeToQString(m_grid->agentType(m_fSite))));
            m_fSite = m_site;
            break;
        }

        default:
        {
            // Kill program - this should never happen
            qFatal("ChargeAgent::decideFuture transport to unhandled site type: %s",
                   qPrintable(Agent::typeToQString(m_grid->agentType(m_fSite))));
            break;
        }
    }
    return;
  }

  void ChargeAgent::completeTick ()
  {
    if (m_site != m_fSite)
      {
        // Are we on a drain site? If so then we have been removed.
        if (m_grid->agentType (m_fSite) == Agent::HoleDrainL)
          {
            m_grid->setAgentAddress(m_site,0);
            m_grid->setAgentType(m_site,Agent::Empty);
            m_world->holeDrainL()->acceptCharge(m_type);
            m_removed = true;
            return;
          }
        if (m_grid->agentType (m_fSite) == Agent::HoleDrainR)
          {
            m_grid->setAgentAddress (m_site,0);
            m_grid->setAgentType(m_site,Agent::Empty);
            m_world->holeDrainR()->acceptCharge(m_type);
            m_removed = true;
            return;
          }

        // Check if another charge snuck into this site before us.
        else if (m_grid->agentAddress(m_fSite))
          {
            // Abort the move - site is now occupied
            m_fSite = m_site;
            return;
          }

        //if (m_site != -1)
        //Vacate the old site
        m_grid->setAgentAddress(m_site,0);
        m_grid->setAgentType(m_site,Agent::Empty);

        // Everything looks good - set this agent to the new site.
        m_site = m_fSite;
        m_grid->setAgentAddress (m_site,this);
        m_grid->setAgentType(m_site,m_type);

        // Update our neighbors
        m_neighbors = m_grid->neighborsSite(m_site);
      }
  }

  inline double ChargeAgent::coulombInteraction (int newSite)
  {
      double p1 = 0;
      double p2 = 0;

      if (m_world->parameters()->useOpenCL)
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
          p1 += m_world->potential()->coulombPotentialElectrons( m_site  );
          p2 += m_world->potential()->coulombPotentialElectrons( newSite );

          // Holes
          p1 += m_world->potential()->coulombPotentialHoles( m_site  );
          p2 += m_world->potential()->coulombPotentialHoles( newSite );

          // Remove self interaction
          p2 -= m_world->interactionEnergies()[1][0][0] * m_charge;

          // Charged defects
          if ( m_world->parameters()->chargedDefects )
          {
              p1 += m_world->potential()->coulombPotentialDefects( m_site  );
              p2 += m_world->potential()->coulombPotentialDefects( newSite );
          }
      }

      if ( m_type == Agent::Electron )
      {
          if ( m_world->holeGrid()->agentType(newSite) == Agent::Hole )
          {
              p2 += 0.5;
          }
      }
      return m_charge * (p2 - p1);
  }
}
