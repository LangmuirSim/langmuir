#include "drainagent.h"
#include "chargeagent.h"
#include "inputparser.h"
#include "logger.h"
#include "rand.h"
#include "world.h"
#include "grid.h"

namespace Langmuir
{

  DrainAgent::DrainAgent(World *world) : Agent(Agent::Empty,world)
  {
      m_acceptedHoles = 0;
      m_acceptedElectrons = 0;
      m_attempts     = 0;
      m_successes    = 0;
      m_site         = 0;
      m_fSite        = 0;
      m_neighbors = QVector<int>() << -1;
      if ( m_world->electronGrid() == 0 )
      {
          qFatal("DrainAgent allocation requires the Grid to exist");
      }
  }

  ElectronDrainAgentLeft::ElectronDrainAgentLeft(World *world) : ElectronDrainAgent(world)
  {
      m_type = Agent::ElectronDrainL;
      Grid &grid = *m_world->electronGrid();
      m_site  = grid.getIndexDrainL();
      m_fSite = m_site;
      grid.setAgentAddress(m_site,this);
      grid.setAgentType(m_site,Agent::ElectronDrainL);
      m_neighbors = grid.sliceIndex(0,1,0,grid.height(),0,grid.depth());
  }

  ElectronDrainAgentRight::ElectronDrainAgentRight(World *world) : ElectronDrainAgent(world)
  {
      m_type = Agent::ElectronDrainR;
      Grid &grid = *m_world->electronGrid();
      m_site  = grid.getIndexDrainR();
      m_fSite = m_site;
      grid.setAgentAddress(m_site,this);
      grid.setAgentType(m_site,Agent::ElectronDrainR);
      m_neighbors = grid.sliceIndex(grid.width()-1,grid.width(),0,grid.height(),0,grid.depth());
  }

  HoleDrainAgentLeft::HoleDrainAgentLeft(World *world) : HoleDrainAgent(world)
  {
      m_type = Agent::HoleDrainL;
      Grid &grid = *m_world->holeGrid();
      m_site  = grid.getIndexDrainL();
      m_fSite = m_site;
      grid.setAgentAddress(m_site,this);
      grid.setAgentType(m_site,Agent::HoleDrainL);
      m_neighbors = grid.sliceIndex(0,1,0,grid.height(),0,grid.depth());
  }

  HoleDrainAgentRight::HoleDrainAgentRight(World *world) : HoleDrainAgent(world)
  {
      m_type = Agent::HoleDrainR;
      Grid &grid = *m_world->holeGrid();
      m_site  = grid.getIndexDrainR();
      m_fSite = m_site;
      grid.setAgentAddress(m_site,this);
      grid.setAgentType(m_site,Agent::HoleDrainR);
      m_neighbors = grid.sliceIndex(grid.width()-1,grid.width(),0,grid.height(),0,grid.depth());
  }


/*
  DrainAgentLeft::DrainAgentLeft(World *world) : DrainAgent(world)
  {
      m_type = Agent::DrainL;
      Grid &EGrid = *m_world->electronGrid();
      Grid &HGrid = *m_world->holeGrid();
      m_site  = EGrid.getIndexDrainL();
      m_fSite = m_site;
      EGrid.setAgentAddress(m_site,this);
      EGrid.setAgentType(m_site,Agent::DrainL);
      HGrid.setAgentAddress(m_site,this);
      HGrid.setAgentType(m_site,Agent::DrainL);
      m_neighbors = EGrid.sliceIndex(0,1,0,EGrid.height(),0,EGrid.depth());
  }

  DrainAgentRight::DrainAgentRight(World *world) : DrainAgent(world)
  {
      m_type = Agent::DrainR;
      Grid &EGrid = *m_world->electronGrid();
      Grid &HGrid = *m_world->holeGrid();
      m_site  = EGrid.getIndexDrainR();
      m_fSite = m_site;
      EGrid.setAgentAddress(m_site,this);
      EGrid.setAgentType(m_site,Agent::DrainR);
      HGrid.setAgentAddress(m_site,this);
      HGrid.setAgentType(m_site,Agent::DrainR);
      m_neighbors = EGrid.sliceIndex(EGrid.width()-1,EGrid.width(),0,EGrid.height(),0,EGrid.depth());
  }*/

  DrainAgent::~DrainAgent()
  {
      qDebug() << *this << "\n";
  }

  void DrainAgent::acceptCharge(Agent::Type type)
  {
      //qDebug() << "accepted charge" << type << "at" << m_type;
      switch ( type )
      {
          case Agent::Electron:
          {
              ++m_acceptedElectrons;
              break;
          }
          case Agent::Hole:
          {
              ++m_acceptedHoles;
              break;
          }
          default:
          {
              qFatal("an unknown agent type has attempted to move onto the drain");
              break;
          }
      }
  }

  bool DrainAgent::tryToAccept(ChargeAgent *charge)
  {
      return false;
  }

  double DrainAgent::energyChange(ChargeAgent *charge)
  {
      /*
      double p1 = m_world->holeGrid()->potential(m_site);
      double p2 = m_world->holeGrid()->potential(site);
      return p2-p1;
      */
      return 0.0;
  }

  bool DrainAgent::shouldAccept(ChargeAgent *charge)
  {
      return m_world->randomNumberGenerator()->randomlyChooseYesWithPercent( m_world->coupling()[charge->type()][m_type] );
      //return m_world->randomNumberGenerator()->randomlyChooseYesWithMetropolisAndCoupling(
      //        energyChange(site),m_world->parameters()->inverseKT,m_world->coupling()[m_type][Agent::Empty] );
  }

  unsigned long int DrainAgent::acceptedCarriers() const
  {
      return m_acceptedElectrons + m_acceptedHoles;
  }

  unsigned long int DrainAgent::acceptedElectrons() const
  {
      return m_acceptedElectrons;
  }

  unsigned long int DrainAgent::acceptedHoles() const
  {
      return m_acceptedHoles;
  }

  unsigned long int DrainAgent::attempts() const
  {
      return m_attempts;
  }

  unsigned long int DrainAgent::successes() const
  {
      return m_successes;
  }

  double DrainAgent::successRate() const
  {
      if ( m_attempts > 0 )
      {
          return double(m_successes)/double(m_attempts)*100.0;
      }
      else
      {
          return 0.0;
      }
  }

  QString DrainAgent::toQString() const
  {
      QString result = Agent::toQString();
      result = result.replace( QRegExp("\\s*\\]\\s*$"), "\n" );
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

} // End Langmuir namespace
