#include "chargeagent.h"
#include "world.h"
#include "grid.h"
#include "rand.h"
#include "inputparser.h"

namespace Langmuir
{
    ChargeAgent::ChargeAgent (World * world, int site):Agent (Agent::Charge, world, site), 
     m_charge (-1), m_removed (false), m_lifetime(0), m_distanceTraveled(0.0), OpenCLID(0)
  {
    //current site and future site are the same
    m_site = m_fSite;

    //make sure this agent is somewhere on the grid
    m_world->grid ()->setAgent (m_site, this);

    //figure out the neighbors of this site
    m_neighbors = m_world->grid ()->neighbors (m_site,m_world->parameters()->hoppingRange);
  }

  ChargeAgent::~ChargeAgent ()
  {
  }

  void ChargeAgent::chooseFuture()
  {
    // Select a proposed transport site at random, but ensure that it is not the source
    do
      {
        m_fSite = m_neighbors[int(m_world->randomNumberGenerator()->random() * (m_neighbors.size() - 1.0e-20))];
      }
    while (m_world->grid()->siteID(m_fSite) == 2); // source siteID
  }

  int ChargeAgent::decideFuture()
  {
    // Increase lifetime in existance
    m_lifetime += 1;

    // Get a pointer to the grid for easy access
    Grid *grid = m_world->grid ();

    // Check the proposed site, return unsucessful if it is the SOURCE, a DEFECT, or another CARRIER
    if ( grid->agent(m_fSite) && grid->siteID (m_fSite) != 3 ) { m_fSite = m_site; return -1; }

    // If the site is the DRAIN, perform drain acceptance probability calculations ( drain.type, drain.barrier )
    if ( grid->siteID(m_fSite) == 3 )
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
    double pd = grid->potential(m_fSite) - grid->potential (m_site);
    pd *= m_charge * m_world->parameters()->elementaryCharge;

    // Add on the Coulomb interaction if it is being included
    if ( m_world->parameters()->interactionCoulomb )
      {
        // get the answer from OpenCL output vector
        if (m_world->parameters()->useOpenCL)
          {
            //obtain coulomb interactions from OpenCL ( defects and carrier interactions )
            //pd += m_world->getOutputHost( clID );
              pd += ( ( m_world->getOutputHost(m_fSite) - m_world->getOutputHost(m_site) - m_charge ) * m_charge * m_world->parameters()->electrostaticPrefactor * m_world->parameters()->elementaryCharge );
          }
        // or calculate it on the CPU
        else
          {
            //obtain coulomb interactions on Host
            pd += this->coulombInteraction (m_fSite);

            //add the interactions from charged defects ( on CPU )
            if (m_world->parameters()->chargedDefects)
            {
             pd += chargedDefects (m_fSite);
            }

           }

        // Add the interactions from charged traps ( this part hasn't been implemented in OpenCL yet )
        if (m_world->parameters()->chargedTraps)
          {
            pd += chargedTraps(m_fSite);
          }
      }

    // Apply metropolis criterion
    if (attemptTransport (pd, couplingConstant(grid->siteID(m_site), grid->siteID(m_fSite)) ))
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
        if (m_world->grid ()->siteID (m_fSite) == 3)
          {
            m_world->grid ()->setAgent (m_site, 0);
            m_removed = true;
            return;
          }

        // Check if another charge snuck into this site before us.
        else if (m_world->grid ()->agent (m_fSite))
          {
            // Abort the move - site is now occupied
            m_fSite = m_site;
            return;
          }

        if (m_site != -1)
          // Vacate the old site
          m_world->grid ()->setAgent (m_site, 0);

        // Everything looks good - set this agent to the new site.
        m_site = m_fSite;
        m_world->grid ()->setAgent (m_site, this);

        // Update our neighbors
        m_neighbors = m_world->grid ()->neighbors (m_site,m_world->parameters()->hoppingRange);
      }
  }

  inline double ChargeAgent::coulombInteraction (int newSite)
  {
    // Pointer to grid
    Grid *grid = m_world->grid ();

    // Reference to charges in simulation
    QList < ChargeAgent * >&charges = *m_world->charges ();

    // Number of charges in simulation
    int chargeSize = charges.size ();

    // Potential before
    double potential1 = 0.0;

    // Potential after
    double potential2 = 0.0;

    //qDebug("size = %5d",chargeSize);
    //qDebug("site = %5d",this->site());
    //qDebug("fsite= %5d",newSite);
    for (int i = 0; i < chargeSize; ++i)
      {
        //qDebug("on i = %5d",i);
        //qDebug("size = %5d",chargeSize);
        //qDebug("site = %5d",this->site());
        //qDebug("fsite= %5d",newSite);
        if (charges[i] != this)
          {

            // Potential at current site from other charges
            int dx = grid->xDistancei (m_site, charges[i]->site ());
            int dy = grid->yDistancei (m_site, charges[i]->site ());
            int dz = grid->zDistancei (m_site, charges[i]->site ());
            //qDebug("i: %5d dx1: %5d dy1: %5d dz1: %5d",i,dx,dy,dz);
            if (dx < m_world->parameters ()->electrostaticCutoff &&
                dy < m_world->parameters ()->electrostaticCutoff &&
                dz < m_world->parameters ()->electrostaticCutoff)
              {
                potential1 +=
                        m_world->interactionEnergies()[dx][dy][dz] * charges[i]->charge();
                  //qDebug("NOW i = %d, array = %e, q1 = %d, q2 = %d, prefactor = %e",i,m_world->interactionEnergies()(dx, dy, dz),charges[i]->charge(),m_charge,m_world->parameters()->electrostaticPrefactor);
              }

            // Potential at new site from other charges
            if (newSite != charges[i]->site ())
              {
                dx = grid->xDistancei (newSite, charges[i]->site ());
                dy = grid->yDistancei (newSite, charges[i]->site ());
                dz = grid->zDistancei (newSite, charges[i]->site ());
                //qDebug("i: %5d dx2: %5d dy2: %5d dz2: %5d",i,dx,dy,dz);
                if (dx < m_world->parameters ()->electrostaticCutoff &&
                    dy < m_world->parameters ()->electrostaticCutoff &&
                    dz < m_world->parameters ()->electrostaticCutoff)
                  {
                    potential2 += m_world->interactionEnergies()[dx][dy][dz] * charges[i]->charge();
                    //qDebug("FUTURE i = %d, array = %e, q1 = %d, q2 = %d, prefactor = %e",i,m_world->interactionEnergies()(dx, dy, dz),charges[i]->charge(),m_charge,m_world->parameters()->electrostaticPrefactor);
                  }
              }
             else
               {
                 // NOTE: this else case should never happen if decideFuture did its job correctly
                 return -1;
               }
          } //else{ qDebug("charge[i]==this"); }
      }
    return m_charge * m_world->parameters ()->electrostaticPrefactor *
      (potential2 - potential1);
  }

  inline double ChargeAgent::chargedDefects (int newSite)
  {
    // Pointer to grid
    Grid *grid = m_world->grid ();

    // Reference to charged defects in the system
    QList < int >&chargedDefects = *m_world->defectSiteIDs ();

    // Potential before
    double potential1 = 0.0;

    // Potential after
    double potential2 = 0.0;

    // Number of defects in the system
    int defectSize (chargedDefects.size ());

    for (int i = 0; i < defectSize; ++i)
      {
        // Potential at current site from charged defects
        int dx = grid->xDistancei (m_site, chargedDefects[i]);
        int dy = grid->yDistancei (m_site, chargedDefects[i]);
        int dz = grid->zDistancei (m_site, chargedDefects[i]);
        if (dx < m_world->parameters ()->electrostaticCutoff &&
            dy < m_world->parameters ()->electrostaticCutoff &&
            dz < m_world->parameters ()->electrostaticCutoff)
          {
            potential1 += m_world->interactionEnergies ()[dx][dy][dz];
          }
        // Potential at new site from charged defects
        if (newSite != chargedDefects[i])
          {
            dx = grid->xDistancei (newSite, chargedDefects[i]);
            dy = grid->yDistancei (newSite, chargedDefects[i]);
            dz = grid->zDistancei (newSite, chargedDefects[i]);
            if (dx < m_world->parameters ()->electrostaticCutoff &&
                dy < m_world->parameters ()->electrostaticCutoff &&
                dz < m_world->parameters ()->electrostaticCutoff)
              {
                potential2 += m_world->interactionEnergies ()[dx][dy][dz];
              }
          }
        else
          {
            return -1;
          }
      }
    return m_charge * m_world->parameters ()->zDefect *
      m_world->parameters ()->electrostaticPrefactor * (potential2 -
                                                        potential1);
  }

  inline double ChargeAgent::chargedTraps (int newSite)
  {
    // Pointer to grid
    Grid *grid = m_world->grid ();

    // Reference to charged traps
    QList < int >&chargedTraps = *m_world->trapSiteIDs ();

    // Potential before
    double potential1 = 0.0;

    // Potential after
    double potential2 = 0.0;

    // Number of charged traps
    int trapSize (chargedTraps.size ());

    for (int i = 0; i < trapSize; ++i)
      {
        // Potential at current site from charged traps
        int dx = grid->xDistancei (m_site, chargedTraps[i]);
        int dy = grid->yDistancei (m_site, chargedTraps[i]);
        int dz = grid->zDistancei (m_site, chargedTraps[i]);
        if (dx < m_world->parameters ()->electrostaticCutoff &&
            dy < m_world->parameters ()->electrostaticCutoff &&
            dz < m_world->parameters ()->electrostaticCutoff &&
            dx != 0 && dy != 0 && dz != 0)
          {
            potential1 += m_world->interactionEnergies ()[dx][dy][dz];
          }
        // Potential at new site from charged traps
        if (newSite != chargedTraps[i])
          {
            dx = grid->xDistancei (newSite, chargedTraps[i]);
            dy = grid->yDistancei (newSite, chargedTraps[i]);
            dz = grid->zDistancei (newSite, chargedTraps[i]);
            if (dx < m_world->parameters ()->electrostaticCutoff &&
                dy < m_world->parameters ()->electrostaticCutoff &&
                dz < m_world->parameters ()->electrostaticCutoff &&
                dx != 0 && dy != 0 && dz != 0)
              {
                potential2 += m_world->interactionEnergies ()[dx][dy][dz];
              }
          }
        else
          {
            return -1;
          }
      }
    return m_charge * m_world->parameters ()->zTrap *
      m_world->parameters ()->electrostaticPrefactor * (potential2 -
                                                        potential1);
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
      if ( m_world->parameters()->chargedDefects )
      {
          if ( m_world->parameters()->chargedTraps )
          {
              return coulombInteraction(m_fSite) + chargedDefects(m_fSite) + chargedTraps(m_fSite);
          }
          return coulombInteraction(m_fSite) + chargedDefects(m_fSite);
      }
      return coulombInteraction(m_fSite);
  }

}
