#include "chargeagent.h"

#include "world.h"
#include "grid.h"

#include <QtCore/QThread>
#include <QtCore/QDebug>

#include <cstdlib>

namespace Langmuir{

  using std::vector;
  using Eigen::Vector2d;

  ChargeAgent::ChargeAgent(World *world, unsigned int site, bool coulombInteraction)
      : Agent(Agent::Charge, world, site), m_charge(-1), m_removed(false),
      m_coulombInteraction(coulombInteraction)
  {
//    qDebug() << "Charge injected into the system.";
    m_site = m_fSite;
    m_world->grid()->setAgent(m_site, this);
    // Update our neighbors
    m_neighbors = m_world->grid()->neighbors(m_site);
  }

  ChargeAgent::~ChargeAgent()
  {
  }

  void ChargeAgent::setCoulombInteraction(bool enabled)
  {
    m_coulombInteraction = enabled;
  }

  unsigned int ChargeAgent::transport()
  {
    // Set up some constants...
    const double q = 1.60217646e-19; // Magnitude of charge on an electron

    // Choose the appropriate site and attempt a move
    Grid *grid = m_world->grid();
//    Vector2d pos1(grid->position(m_site));

    // Now we select the proposed transport site at random
    unsigned int newSite = m_neighbors[int(m_world->random()
                                           *(m_neighbors.size()-1.0e-20))];

    // Check if the proposed site to move to is already occupied
    if (grid->agent(newSite) && grid->siteID(newSite) != 3)
      return -1;

//    qDebug() << "Potential of charges:" << potential1 << potential2
//        << "Difference" << potential2 - potential1;

    // Now to add on the background potential - from the applied field
    // A few electrons only perturb the potential by ~1e-20 or so.
    double pd = grid->potential(newSite) - grid->potential(m_site);
    pd *= m_charge * q;
//    qDebug() << "Potential difference neglecting charge:" << pd;

    // Add on the Coulomb interaction if it is being included
    if (m_coulombInteraction)
      pd += this->coulombInteraction(newSite);

//    qDebug() << "Potential difference with charges:" << pd;

    // Now attempt to move the charge
    double coupling = couplingConstant(grid->siteID(m_site),
                                       grid->siteID(newSite));
    if (attemptTransport(pd, coupling)) {
      m_fSite = newSite;
      return m_fSite;
    }

    return -1;
  }

  void ChargeAgent::completeTick()
  {
    if (m_site != m_fSite) {
//      Vector2d pos1(m_world->grid()->position(m_site));
//      Vector2d pos2(m_world->grid()->position(m_fSite));
/*      qDebug() << "Charge move:"
          << pos1.x() << pos1.y()
          << "->"
          << pos2.x() << pos2.y()
          << "\n" << m_site << "->" << m_fSite; */
/*      if (pos1.x() < 0.0 && pos1.y() < 0.0) {
        qDebug() << "Charge injected into the system.";
        m_site = m_fSite;
        m_world->grid()->setAgent(m_site, this);
        // Update our neighbors
        m_neighbors = m_world->grid()->neighbors(m_site);
        return;
      }
      else */ //if (fabs(pos2.x() - pos1.x()) > 1.01 ||
              // fabs(pos2.y() - pos1.y()) > 1.01) {
       // qDebug() << "WARNING!!! Error has occurred!!! Move too large.";
      //}

      // Are we on a drain site? If so then we have been removed.
      if (m_world->grid()->siteID(m_fSite) == 3) {
        m_world->grid()->setAgent(m_site, 0);
        m_removed = true;
//        qDebug() << "Charge moved to drain site - remove!";
        return;
      }
      // Check if another charge snuck into this site before us
      else if (m_world->grid()->agent(m_fSite)) {
        // Abort the move - site is now occupied
        m_fSite = m_site;
        return;
      }

      if (m_site != errorValue) // Vacate the old site
        m_world->grid()->setAgent(m_site, 0);
      // Everything looks good - site this agent to the new site too.
      m_site = m_fSite;
      m_world->grid()->setAgent(m_site, this);
      // Update our neighbors
      m_neighbors = m_world->grid()->neighbors(m_site);
    }
  }

  inline double ChargeAgent::coulombInteraction(unsigned int newSite)
  {
    const double q = 1.60217646e-19; // Magnitude of charge on an electron
    // Prefactor for force calculations q / 4 pi epsilon with a 1e-9 for m -> nm
    // Using a relative permittivity of 3.5 for now - should be configurable
    const double q4pe = q / (4.0*M_PI*8.854187817e-12 * 3.5 * 1e-9);

    // Cutting off interaction energies at 50nm
    int cutoff = 50;

    Grid *grid = m_world->grid();
//    Vector2d pos1(grid->position(m_site));
    // Get the position of the proposed site
//    Vector2d pos2(grid->position(newSite));

    // Figure out the potential of the site we are on
    QList<ChargeAgent *> &charges = *m_world->charges();
    double potential1(0.0), potential2(0.0);
    int chargeSize(charges.size());
    for (int i = 0; i < chargeSize; ++i) {
      if (charges[i] != this) {
        // Potential at current site from other charges
        int dx = grid->xDistancei(m_site, charges[i]->site());
        int dy = grid->yDistancei(m_site, charges[i]->site());
        if (dx < 1 && dy < 1) {
          qDebug() << "Distance in charge agent too small!"
              << dx << dy << m_site << charges[i]->site();
          qDebug() << "Objects:" << this << charges[i];
          std::exit(1);
        }
        if (dx < cutoff && dy < cutoff) {
          potential1 += (*m_world->interactionEnergies())(dy, dx)
                        * charges[i]->charge();
        }
        // Potential at new site from other charges
        if (newSite != charges[i]->site()) {
          dx = grid->xDistancei(newSite, charges[i]->site());
          dy = grid->yDistancei(newSite, charges[i]->site());
          if (dx < cutoff && dy < cutoff) {
            potential2 += (*m_world->interactionEnergies())(dy, dx)
                          * charges[i]->charge();
          }
        }
        else {
//          qDebug() << "This site is already occupied - reject.";
          return -1;
        }
      }
    }
    return m_charge * q4pe * (potential2 - potential1);
  }

  inline double ChargeAgent::couplingConstant(short id1, short id2)
  {
    return (*m_world->coupling())(id1, id2);
  }

  inline bool ChargeAgent::attemptTransport(double pd, double coupling)
  {
    // Now to decide if the hop will be made - using a simple exponential energy
    // change acceptance now.

    //Define the Boltzmann constant
    const double k = 1.3806504e-23;  // Boltzmann's constant in J/K

    // Now define  a variable called kTinv using the variable temperatureKelvin.
    // This should allow for the temperature to be set to a specific value or
    // to be a working variable.

    double kTinv = 1 / (k * temperatureKelvin);  // 1/kBT calcualted with a user defined
                                                 // temperature.

    double randNumber = m_world->random();
//    qDebug() << "Deciding on whether to accept move..." << pd << exp(-pd * kTinv)
//        << randNumber;
    if (pd > 0.0) {
//      qDebug() << "Method 1:" << pd << exp(-pd * kTinv) << randNumber;
      if ((coupling * exp(-pd * kTinv)) > randNumber) {
//        qDebug() << "Charge move accepted..." << m_fSite;
        return true;
      }
    }
    else if (coupling > randNumber) {
//      qDebug() << "Method 2:" << pd << randNumber;
//      qDebug() << "Charge move accepted (2)..." << m_fSite;
      return true;
    }
    return false;
  }

}

