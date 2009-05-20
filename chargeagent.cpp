#include "chargeagent.h"

#include "world.h"
#include "grid.h"

#include <QtCore/QThread>
#include <QtCore/QDebug>

namespace Langmuir{

  using std::vector;
  using Eigen::Vector2d;

  ChargeAgent::ChargeAgent(World *world, unsigned int site) :
      Agent(world, site), m_charge(-1)
  {
  }

  ChargeAgent::~ChargeAgent()
  {
  }

  unsigned int ChargeAgent::transport()
  {
    qDebug() << "ChargeAgent in thread" << QThread::currentThread();

    const double q = 1.60217646e-19;
    // Prefactor for force calculations q / 4 pi epsilon with a 1e-9 for m -> nm
    const double q4pe = q / (4.0*M_PI*8.854187817e-12 * 1e-9);
    // Choose the appropriate site and attempt a move
    Grid *grid = m_world->grid();
    Vector2d pos(grid->position(m_site));

    // Figure out the potential of the site we are on
    vector<ChargeAgent *> &charges = *m_world->charges();
    double potential(0.0);
    for (unsigned int i = 0; i < charges.size(); ++i) {
      if (charges[i] != this) {
        double distance = (pos - grid->position(charges[i]->site())).norm();
        qDebug() << "Distance of 1->2:" << distance << charges[i]->charge();
        // Potential
        potential += (charges[i]->charge() * q) / distance;
      }
    }
    potential *= m_charge * q4pe;
    qDebug() << "Potential of charges:" << potential;
    // Now to add on the background potential - from the applied field
    // A few electrons only perturb the potential by ~1e-20 or so.
    potential += grid->potential(m_site);

    qDebug() << "E-field:" << m_world->eField() << endl
        << "Potential:" << grid->potential(m_site) << endl
        << "Potential2:" << potential;

    // Change in potential energy is -qEd (d = distance)
    return -1;
  }

  void ChargeAgent::completeTick()
  {
    if (m_site != m_fSite) {
      if (m_site != errorValue)
        m_world->grid()->setAgent(m_site, 0);
      m_site = m_fSite;
      m_world->grid()->setAgent(m_site, this);
      m_neighbors = m_world->grid()->neighbors(m_site);
    }
  }

}

