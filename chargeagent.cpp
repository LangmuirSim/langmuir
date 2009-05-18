#include "chargeagent.h"

#include "world.h"
#include "grid.h"

#include <iostream>

namespace Langmuir{

  using std::cout;
  using std::endl;
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
    const double q = 1.60217646e-19;
    // Prefactor for force calculations 4 / 4 pi epsilon with a 1e9 for nm -> m
    const double q4pe = (1.60217646e-19*1e18) / (4.0*M_PI*8.854187817e-12);
    // Choose the appropriate site and attempt a move
    Grid *grid = m_world->grid();
    Vector2d pos(grid->position(m_site));

    Vector2d force(m_world->eField() * m_charge * q, 0.0);

    // Figure out the force on the charge
    vector<ChargeAgent *> &charges = *m_world->charges();
    Vector2d sum(0.0, 0.0);
    double potential(0.0);
    for (unsigned int i = 0; i < charges.size(); ++i) {
      if (charges[i] == this)
        continue;
      Vector2d diff = pos - grid->position(charges[i]->site());
      // Force
      sum += (m_charge * q * diff.normalized()) / diff.squaredNorm();
      // Potential
      potential += (m_charge * q) / diff.norm();
    }
    sum *= q4pe;
    force += sum;

    cout << "E-field: " << m_world->eField() << endl
         << "force on charge; " << force.x() << ", " << force.y() << endl;

    // Change in potential energy is -qEd (d = distance)

/*    cout << "Position: " << pos.x() << ", " << pos.y() << endl;
    for (unsigned int i = 0; i < m_neighbors.size(); ++i) {
      Vector2d diff = pos - grid->position(m_neighbors[i]);
      cout << "Diff between neighbor: " << diff.x() << ", " << diff.y()
           << " " << diff.norm() << " " <<  m_neighbors[i] << endl;
    }
*/
    return -1;
  }

  void ChargeAgent::completeTick()
  {
//    cout << "Complete tick called. " << m_site << " " << m_fSite << endl;
    if (m_site != m_fSite) {
      if (m_site != errorValue)
        m_world->grid()->setAgent(m_site, 0);
      m_site = m_fSite;
      m_world->grid()->setAgent(m_site, this);
      m_neighbors = m_world->grid()->neighbors(m_site);
//      cout << "Called setAgent in grid..." << m_site << endl;
    }
  }

}

