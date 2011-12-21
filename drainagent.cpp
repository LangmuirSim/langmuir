#include "drainagent.h"
#include "chargeagent.h"
#include "inputparser.h"
#include "rand.h"
#include "world.h"
#include "grid.h"

namespace Langmuir
{

  DrainAgent::DrainAgent(Agent::Type type, World *world, int site) : Agent(type, world, site)
  {
      if ( type != Agent::DrainL && type != Agent::DrainR )
      {
          qFatal("DrainAgent must have Agent:Type equal to Agent::DrainL or Agent::DrainR");
      }
      m_site = 0;
      m_neighbors.clear();
      m_acceptedHoles = 0;
      m_acceptedElectrons = 0;
  }

  DrainAgent::~DrainAgent()
  {
      //qDebug() << "holes" << m_acceptedHoles << "electrons" << m_acceptedElectrons << m_type;
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

  bool DrainAgent::attemptTransport(ChargeAgent *charge)
  {
      return m_world->randomNumberGenerator()->randomlyChooseYesWithPercent( m_world->coupling()[charge->type()][m_type] );
  }

} // End Langmuir namespace
