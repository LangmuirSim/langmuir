#include "openclhelper.h"
#include "chargeagent.h"
#include "inputparser.h"
#include "simulation.h"
#include "potential.h"
#include "world.h"
#include "grid.h"
#include "rand.h"


namespace Langmuir
{
    ChargeAgent::ChargeAgent (World * world, int site, bool isHole):Agent (Agent::Charge, world, site),
     m_charge (-1), m_removed (false), m_lifetime(0), m_distanceTraveled(0.0), m_openClID(0)
  {
    //current site and future site are the same
    m_site = m_fSite;

    if ( isHole )
    {
        m_grid = m_world->holeGrid();
    }
    else
    {
        m_grid = m_world->electronGrid();
    }
    //make sure this agent is somewhere on the grid
    m_grid->setAgent (m_site, this);

    //figure out the neighbors of this site
    m_neighbors = m_grid->neighborsSite(m_site);
  }

  ChargeAgent::~ChargeAgent ()
  {
  }

  void ChargeAgent::chooseFuture()
  {
    // Select a proposed transport site at random, but ensure that it is not the source
    do
      {
        m_fSite = m_neighbors[ m_world->randomNumberGenerator()->integer(0,m_neighbors.size()-1) ];
      }
    while (m_grid->siteID(m_fSite) == 2); // source siteID
  }

  int ChargeAgent::decideFuture()
  {
    // Increase lifetime in existance
    m_lifetime += 1;

    // Check the proposed site, return unsucessful if it is the SOURCE, a DEFECT, or another CARRIER
    if ( m_grid->agent(m_fSite) && m_grid->siteID (m_fSite) != 3 ) { m_fSite = m_site; return -1; }

    // If the site is the DRAIN, perform drain acceptance probability calculations ( drain.type, drain.barrier )
    if ( m_grid->siteID(m_fSite) == 3 )
    {
        switch ( m_world->parameters()->drainType )
        {
            case 0: // Constant case
            {
                // Do not accept 100*sourceBarrier percent of the time
                if(m_world->randomNumberGenerator()->random() <= m_world->parameters()->drainBarrier)
                {
                 m_fSite = m_site;
                 return -1;
                }
                // accept the charge carrier
                else
                {
                 m_distanceTraveled += 1.0;
                 return m_fSite;
                }
                break;
            }

            case 1: // Broken case
            {
                // The broken case allowed the potential of the drain to be used to calculate acceptance
                // ...coulomb interactions between the carrier and the drain were also calculated (undefined)
                break;
            }

            default:
            {
                qFatal("unknown drain acceptance probability calculation type encountered");
                return -1;
                break;
            }
        }
    }

    // Now to add on the background potential - from the applied field
    // A few electrons only perturb the potential by ~1e-20 or so.
    double pd = m_grid->potential(m_fSite) - m_grid->potential (m_site);
    pd *= m_charge;// * m_world->parameters()->elementaryCharge;

    // Add on the Coulomb interaction if it is being included    
    if ( m_world->parameters()->interactionCoulomb )
      {
            pd += this->coulombInteraction (m_fSite);
      }

    // Apply metropolis criterion
    if (attemptTransport (pd, couplingConstant(m_grid->siteID(m_site), m_grid->siteID(m_fSite)) ))
      {
        // Set the future site is sucessful
        m_distanceTraveled += 1.0;
        return m_fSite;
      }

    // Return -1 if transport unsucessful
    m_fSite = m_site;
    return -1;
  }

  int ChargeAgent::transport ()
  {
    // Select a proposed transport site
    chooseFuture();

    // decide if the future site is ok to transport to
    return decideFuture();
  }

  void ChargeAgent::completeTick ()
  {
    if (m_site != m_fSite)
      {

        // Are we on a drain site? If so then we have been removed.
        // Apparently charge carriers are people too.
        if (m_grid->siteID (m_fSite) == 3)
          {
            m_grid->setAgent (m_site, 0);
            m_removed = true;
            return;
          }

        // Check if another charge snuck into this site before us.
        else if (m_grid->agent (m_fSite))
          {
            // Abort the move - site is now occupied
            m_fSite = m_site;
            return;
          }

        if (m_site != -1)
          // Vacate the old site
          m_grid->setAgent (m_site, 0);

        // Everything looks good - set this agent to the new site.
        m_site = m_fSite;
        m_grid->setAgent (m_site, this);

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
          p1 = m_world->opencl()->getOutputHost(m_openClID);
          p2 = m_world->opencl()->getOutputHostFuture(m_openClID);

          // Remove self interaction
          p2 -= m_charge;

          // Convert to Joules
          p1 = p1 * m_world->parameters()->electrostaticPrefactor;
          p2 = p2 * m_world->parameters()->electrostaticPrefactor;
      }
      else
      {
          // Electrons
          p1 += m_world->potential()->coulombEnergyElectrons( m_site  );
          p2 += m_world->potential()->coulombEnergyElectrons( newSite );

          // Holes
          p1 += m_world->potential()->coulombEnergyHoles( m_site  );
          p2 += m_world->potential()->coulombEnergyHoles( newSite );

          // Remove self interaction
          p2 -= m_world->interactionEnergies()[1][0][0] * m_charge;

          // Charged defects
          if ( m_world->parameters()->chargedDefects )
          {
              p1 += m_world->potential()->coulombEnergyDefects( m_site  );
              p2 += m_world->potential()->coulombEnergyDefects( newSite );
          }
      }
      return m_charge * (p2 - p1);
  }

  inline double ChargeAgent::couplingConstant (short id1, short id2)
  {
    return m_world->coupling()[id1][id2];
  }

  inline bool ChargeAgent::attemptTransport (double pd, double coupling)
  {
    double randNumber =m_world->randomNumberGenerator()->random();
    if (pd > 0.0)
      {
        if ((coupling * exp (-pd * m_world->parameters ()->inverseKT)) >
            randNumber)
          {
            return true;
          }
      }
    else if (coupling > randNumber)
      {
        return true;
      }
    return false;
  }

  double ChargeAgent::interaction()
  {
      return coulombInteraction(m_fSite);
  }

}
