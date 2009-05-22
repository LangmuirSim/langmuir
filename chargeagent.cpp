#include "chargeagent.h"

#include "world.h"
#include "grid.h"

#include <QtCore/QThread>
#include <QtCore/QDebug>

#include <cstdlib>

namespace Langmuir{

  using std::vector;
  using Eigen::Vector2d;

  ChargeAgent::ChargeAgent(World *world, unsigned int site) :
      Agent(Agent::Charge, world, site), m_charge(-1)
  {
    qDebug() << "Charge injected into the system.";
    m_site = m_fSite;
    m_world->grid()->setAgent(m_site, this);
    // Update our neighbors
    m_neighbors = m_world->grid()->neighbors(m_site);
  }

  ChargeAgent::~ChargeAgent()
  {
  }

  unsigned int ChargeAgent::transport()
  {
    // Set up some constants...
    const double q = 1.60217646e-19; // Magnitude of charge on an electron
    const double kTinv = 1.0 / (1.3806504e-23 * 300); // inverse of kBT at 300K
    // Prefactor for force calculations q / 4 pi epsilon with a 1e-9 for m -> nm
    const double q4pe = q / (4.0*M_PI*8.854187817e-12 * 1e-9);
    // Choose the appropriate site and attempt a move
    Grid *grid = m_world->grid();
    Vector2d pos1(grid->position(m_site));

    // Now we select the proposed transport site at random
    unsigned int newSite = int(m_world->random() * (m_neighbors.size()-1.0e-20));
    qDebug() << "Neighbors:" << m_neighbors.size() << "newSite:" << newSite;
    Vector2d pos2(grid->position(m_neighbors[newSite]));

    // Check if the proposed site to move to is already occupied
//    if (grid->agent(m_neighbors[newSite]))
//      return -1;

    // Figure out the potential of the site we are on
    vector<ChargeAgent *> &charges = *m_world->charges();
    double potential1(0.0), potential2(0.0);
    for (unsigned int i = 0; i < charges.size(); ++i) {
      if (charges[i] != this) {
        // Potential at current site from other charges
        double distance = (pos1 - grid->position(charges[i]->site())).norm();
        if (fabs(distance) < 0.001) {
          qDebug() << "Distance in charge agent too small!"
              << distance << m_site << charges[i]->site();
          qDebug() << "Objects:" << this << charges[i];
          std::exit(1);
        }
        potential1 += (charges[i]->charge() * q) / distance;
        // Potential at new site from other charges
        if (m_neighbors[newSite] != charges[i]->site()) {
          distance = (pos2 - grid->position(charges[i]->site())).norm();
          potential2 += (charges[i]->charge() * q) / distance;
        }
        else {
//          qDebug() << "This site is already occupied - reject.";
          return -1;
        }
      }
    }
    potential1 *= m_charge * q4pe;
    potential2 *= m_charge * q4pe;
//    qDebug() << "Potential of charges:" << potential1 << potential2
//        << "Difference" << potential2 - potential1;

    // Now to add on the background potential - from the applied field
    // A few electrons only perturb the potential by ~1e-20 or so.
    double pd = grid->potential(m_neighbors[newSite]) - grid->potential(m_site);
    pd *= m_charge * q;
    qDebug() << "Potential difference neglecting charge:" << pd;
    pd += potential2 - potential1;
    qDebug() << "Potential difference with charges:" << pd;

//    pd *= m_charge;

    // Now to decide if the hop will be made - using a simple exponential energy
    // change acceptance now.
    double randNumber = m_world->random();
    qDebug() << "Deciding on whether to accept move..." << pd << exp(-pd * kTinv)
        << randNumber;
    if (pd > 0.0) {
      if (exp(-pd * kTinv) < randNumber) {
        m_fSite = m_neighbors[newSite];
        qDebug() << "Charge move accepted..." << m_fSite;
        return m_neighbors[newSite];
      }
    }
    else if (0.55 < randNumber) {
      m_fSite = m_neighbors[newSite];
      qDebug() << "Charge move accepted (2)..." << m_fSite;
      return m_neighbors[newSite];
    }
//    else {
//      qDebug() << "Charge move rejected - no move this time...";
//    }

    // Change in potential energy is -qEd (d = distance)
    return -1;
  }

  void ChargeAgent::completeTick()
  {
    if (m_site != m_fSite) {
      Vector2d pos1(m_world->grid()->position(m_site));
      Vector2d pos2(m_world->grid()->position(m_fSite));
      qDebug() << "Charge move:"
          << pos1.x() << pos1.y()
          << "->"
          << pos2.x() << pos2.y();
/*      if (pos1.x() < 0.0 && pos1.y() < 0.0) {
        qDebug() << "Charge injected into the system.";
        m_site = m_fSite;
        m_world->grid()->setAgent(m_site, this);
        // Update our neighbors
        m_neighbors = m_world->grid()->neighbors(m_site);
        return;
      }
      else */ if (fabs(pos2.x() - pos1.x()) > 1.01 ||
               fabs(pos2.y() - pos1.y()) > 1.01) {
        qDebug() << "WARNING!!! Error has occurred!!! Move too large.";
      }

      // Check if another charge snuck into this site before us
      if (m_world->grid()->agent(m_fSite)) {
        // Abort the move - site is now occupied
        m_fSite = m_site;
        return;
      }

      if (m_site != errorValue) // Vacate the old site
        m_world->grid()->setAgent(m_site, 0);
      m_site = m_fSite;
      m_world->grid()->setAgent(m_site, this);
      // Update our neighbors
      m_neighbors = m_world->grid()->neighbors(m_site);
    }
  }

}

