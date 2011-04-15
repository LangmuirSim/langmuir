#include "chargeagent.h"
#include "world.h"
#include "grid.h"
#include "inputparser.h"
#include <QtCore/QThread>
#include <QtCore/QDebug>

#include <cstdlib>

namespace Langmuir
{

  using std::vector;
  using Eigen::Vector3d;

    ChargeAgent::ChargeAgent (World * world, unsigned int site):Agent (Agent::Charge, world, site), 
     m_charge (-1), m_removed (false), m_lifetime(0), m_distanceTraveled(0.0)
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

  unsigned int ChargeAgent::transport ()
  {
    // Increase lifetime in existance
    m_lifetime += 1;

    // Get a pointer to the grid for easy access
    Grid *grid = m_world->grid ();

    // Select a proposed transport site at random, but ensure that it is not the source
    unsigned int newSite;
    do
      {
        newSite =
          m_neighbors[int
                      (m_world->random () * (m_neighbors.size () - 1.0e-20))];
      }
    while (grid->siteID (newSite) == 2);        // source siteID

    // Check the proposed site, return unsucessful if it is the SOURCE, a DEFECT, or another CARRIER
    if (grid->agent (newSite) && grid->siteID (newSite) != 3)
      return -1;

    // If the site is the DRAIN, accept with a constant probability
    if ( grid->siteID (newSite) == 3 )
    {
     // Do not accept 100*sourceBarrier percent of the time
     if(m_world->random() <= m_world->parameters()->sourceBarrier)
     {
      return -1;
     }
     // accept the charge carrier
     else
     {
      m_fSite = newSite;
      m_distanceTraveled += 1.0;
      return m_fSite;
     }
    }

    // Now to add on the background potential - from the applied field
    // A few electrons only perturb the potential by ~1e-20 or so.
    double pd = grid->potential (newSite) - grid->potential (m_site);
    pd *= m_charge * m_world->parameters ()->elementaryCharge;

    // Add on the Coulomb interaction if it is being included
    if (m_world->parameters ()->coulomb)
      pd += this->coulombInteraction (newSite);
    // Add the interactions from charged defects
    if (m_world->parameters ()->chargedDefects)
      pd += chargedDefects (newSite);
    // Add the interactions from charged traps
    if (m_world->parameters ()->chargedTraps)
      pd += chargedTraps (newSite);
    // Get the coupling constant
    double coupling =
      couplingConstant (grid->siteID (m_site), grid->siteID (newSite));

    // Get the temperature
    double T = m_world->parameters ()->temperatureKelvin;

    // Apply metropolis criterion
    if (attemptTransport (pd, coupling, T))
      {
        // Set the future site is sucessful
        m_fSite = newSite;
        m_distanceTraveled += 1.0;
        return m_fSite;
      }

    // Return -1 if transport unsucessful
    return -1;
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

        if (m_site != errorValue)
          // Vacate the old site
          m_world->grid ()->setAgent (m_site, 0);

        // Everything looks good - set this agent to the new site.
        m_site = m_fSite;
        m_world->grid ()->setAgent (m_site, this);

        // Update our neighbors
        m_neighbors = m_world->grid ()->neighbors (m_site,m_world->parameters()->hoppingRange);
      }
  }

  inline double ChargeAgent::coulombInteraction (unsigned int newSite)
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

    for (int i = 0; i < chargeSize; ++i)
      {

        if (charges[i] != this)
          {

            // Potential at current site from other charges
            int dx = grid->xDistancei (m_site, charges[i]->site ());
            int dy = grid->yDistancei (m_site, charges[i]->site ());
            int dz = grid->zDistancei (m_site, charges[i]->site ());
            if (dx < m_world->parameters ()->electrostaticCutoff &&
                dy < m_world->parameters ()->electrostaticCutoff &&
                dz < m_world->parameters ()->electrostaticCutoff)
              {
                potential1 +=
                  m_world->interactionEnergies ()(dx, dy,
                                                  dz) * charges[i]->charge ();
              }

            // Potential at new site from other charges
            if (newSite != charges[i]->site ())
              {
                dx = grid->xDistancei (newSite, charges[i]->site ());
                dy = grid->yDistancei (newSite, charges[i]->site ());
                dz = grid->zDistancei (newSite, charges[i]->site ());
                if (dx < m_world->parameters ()->electrostaticCutoff &&
                    dy < m_world->parameters ()->electrostaticCutoff &&
                    dz < m_world->parameters ()->electrostaticCutoff)
                  {
                    potential2 +=
                      m_world->interactionEnergies ()(dx, dy,
                                                      dz) *
                      charges[i]->charge ();
                  }
              }
            else
              {
                return -1;
              }
          }
      }
    return m_charge * m_world->parameters ()->electrostaticPrefactor *
      (potential2 - potential1);
  }

  inline double ChargeAgent::chargedDefects (unsigned int newSite)
  {
    // Pointer to grid
    Grid *grid = m_world->grid ();

    // Reference to charged defects in the system
    QList < unsigned int >&chargedDefects = *m_world->chargedDefects ();

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
            potential1 += m_world->interactionEnergies ()(dx, dy, dz);
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
                potential2 += m_world->interactionEnergies ()(dx, dy, dz);
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

  inline double ChargeAgent::chargedTraps (unsigned int newSite)
  {
    // Pointer to grid
    Grid *grid = m_world->grid ();

    // Reference to charged traps
    QList < unsigned int >&chargedTraps = *m_world->chargedTraps ();

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
            potential1 += m_world->interactionEnergies ()(dx, dy, dz);
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
                potential2 += m_world->interactionEnergies ()(dx, dy, dz);
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
    return (*m_world->coupling ())(id1, id2);
  }

  inline bool ChargeAgent::attemptTransport (double pd, double coupling,
                                             double T)
  {
    double randNumber = m_world->random ();
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

}
