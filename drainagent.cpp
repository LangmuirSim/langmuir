#include "drainagent.h"
#include "chargeagent.h"
#include "inputparser.h"
#include "cubicgrid.h"
#include "logger.h"
#include "rand.h"
#include "world.h"

namespace Langmuir
{

  DrainAgent::DrainAgent(World * world, double potential, double rate, int tries)
      : FluxAgent(Agent::Drain,world,potential,rate,tries)
  {
  }

  ElectronDrainAgent::ElectronDrainAgent(World * world, int site, double potential, double rate, int tries)
      : DrainAgent(world, potential, rate, tries)
  {
      m_grid  = m_world->electronGrid();
      initializeSite(site);
  }

  ElectronDrainAgent::ElectronDrainAgent(World * world, Grid::CubeFace cubeFace, double potential, double rate, int tries)
      : DrainAgent(world, potential, rate, tries)
  {
      m_grid  = m_world->electronGrid();
      initializeSite(cubeFace);
  }

  HoleDrainAgent::HoleDrainAgent(World * world, int site, double potential, double rate, int tries)
      : DrainAgent(world, potential, rate, tries)
  {
      m_grid  = m_world->holeGrid();
      initializeSite(site);
  }

  HoleDrainAgent::HoleDrainAgent(World * world, Grid::CubeFace cubeFace, double potential, double rate, int tries)
      : DrainAgent(world, potential, rate, tries)
  {
      m_grid  = m_world->holeGrid();
      initializeSite(cubeFace);
  }

  /*
  void DrainAgent::acceptCharge(Agent::Type type)
  {
      m_successes++;
  }

  bool DrainAgent::tryToAccept(ChargeAgent *charge)
  {
      return false;
  }*/

  double ElectronDrainAgent::energyChange(int site)
  {
      double p1 = m_potential;
      double p2 = m_grid->potential(site);
      return p1-p2; // its backwards because q=-1 and dE = q*(p2-p1) = p1-p2
  }

  double HoleDrainAgent::energyChange(int site)
  {
      double p1 = m_potential;
      double p2 = m_grid->potential(site);
      return p2-p1;
  }

  /*
  QString DrainAgent::toQStringEnd() const
  {
      QString result;
      QTextStream stream(&result);

      int w = 20;
      int p =  2;

      stream << QString("%1 %2 %3 %4 %5")
                .arg(      "accepted(e+h):", w        )
                .arg(                    "", w        )
                .arg(                    "", w        )
                .arg(    acceptedCarriers(), w        )
                .arg('\n');

      stream << QString("%1 %2 %3 %4 %5")
                .arg(        "accepted(e):", w        )
                .arg(                    "", w        )
                .arg(                    "", w        )
                .arg(   acceptedElectrons(), w        )
                .arg('\n');

      stream << QString("%1 %2 %3 %4 %5")
                .arg(       "accepted(h):", w        )
                .arg(                   "", w        )
                .arg(                   "", w        )
                .arg(      acceptedHoles(), w        )
                .arg('\n');

      stream << QString("%1 %2 %3 %4 %5")
                .arg(           "rate:", w        )
                .arg(       successes(), w        )
                .arg(        attempts(), w        )
                .arg(     successRate(), w, 'f', p)
                .arg('\n');

      QStringList hdc = m_world->logger()->couplingStringList(Agent::Hole,m_type);
      QStringList edc = m_world->logger()->couplingStringList(Agent::Electron,m_type);

      stream << QString("%1 %2 %3 %4 %5")
                .arg(       "coupling:", w        )
                .arg(            hdc[0], w        )
                .arg(            hdc[1], w        )
                .arg(            hdc[2], w        )
                .arg('\n');

      stream << QString("%1 %2 %3 %4 %5")
                .arg(       "coupling:", w        )
                .arg(            edc[0], w        )
                .arg(            edc[1], w        )
                .arg(            edc[2], w        )
                .arg(']',1);

      return result;
  }
  */

}
