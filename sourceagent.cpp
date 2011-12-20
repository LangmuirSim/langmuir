#include "sourceagent.h"
#include "chargeagent.h"
#include "world.h"
#include "grid.h"
#include "rand.h"
#include "inputparser.h"

namespace Langmuir
{
  int SourceAgent::m_maxElectrons = 0;
  int SourceAgent::m_maxHoles = 0;

  SourceAgent::SourceAgent(Agent::Type type, World * world, int site) :
    Agent(type,world,site)
  {
    if ( type != Agent::SourceL && type != Agent::SourceR )
    {
        qFatal("SourceAgent must have Agent:Type equal to Agent::SourceL or Agent::SourceR");
    }
    m_site = 0;
    m_neighbors.clear();
    m_maxElectrons = m_world->parameters()->chargePercentage*double(m_world->electronGrid()->volume());
    m_maxHoles = m_world->parameters()->chargePercentage*double(m_world->electronGrid()->volume());
  }

  SourceAgent::~SourceAgent()
  {
      qDebug() << m_type;
  }

  int SourceAgent::carrierCount()
  {
      return m_world->electrons()->size() + m_world->holes()->size();
  }

  int SourceAgent::electronCount()
  {
      return m_world->electrons()->size();
  }

  int SourceAgent::holeCount()
  {
      return m_world->holes()->size();
  }

  int SourceAgent::maxCharges()
  {
      return maxElectrons() + maxHoles();
  }

  int SourceAgent::maxElectrons()
  {
      return m_maxElectrons;
  }

  int SourceAgent::maxHoles()
  {
      return m_maxHoles;
  }

  bool SourceAgent::validToInjectHoleAtSite(int site)
  {
      if ( m_world->holeGrid()->agentType(site) != Agent::Empty ||
           m_world->holeGrid()->agentAddress(site) != 0 ||
           site >= m_world->holeGrid()->volume() )
      {
          return false;
      }
      return true;
  }

  bool SourceAgent::validToInjectElectronAtSite(int site)
  {
      if ( m_world->electronGrid()->agentType(site) != Agent::Empty ||
           m_world->electronGrid()->agentAddress(site) != 0 ||
           site >= m_world->electronGrid()->volume() )
      {
          return false;
      }
      return true;
  }

  void SourceAgent::injectHoleAtSite(int site)
  {
      ChargeAgent *charge = new ChargeAgent(Agent::Hole,m_world,site);
      m_world->holes()->push_back(charge);
  }

  void SourceAgent::injectElectronAtSite(int site)
  {
      ChargeAgent *charge = new ChargeAgent(Agent::Electron,m_world,site);
      m_world->electrons()->push_back(charge);
  }

  void SourceAgent::shouldInject(Agent::Type type, int site)
  {

  }

  int SourceAgent::randomSiteID()
  {
      return m_world->randomNumberGenerator()->integer(0,m_world->electronGrid()->volume()-1);
  }

  bool SourceAgent::seedHole()
  {
      int site = randomSiteID();
      if ( validToInjectHoleAtSite(site) )
      {
          injectHoleAtSite(site);
          return true;
      }
      return false;
  }

  bool SourceAgent::seedElectron()
  {
      int site = randomSiteID();
      if ( validToInjectElectronAtSite(site) )
      {
          injectElectronAtSite(site);
          return true;
      }
      return false;
  }

  /*
  inline double SourceAgent::coulombInteraction(int newSite)
  {
    // Pointer to grid
    Grid *grid = m_world->electronGrid();

    // Reference to charges in simulation
    QList < ChargeAgent * >&charges = *m_world->electrons();

    // Number of charges in simulation
    int chargeSize = charges.size();

    // Coulomb Potential
    double potential = 0.0;

    //int count = 0;

    for(int i = 0; i < chargeSize; ++i)
      {
        // Potential at proposed site from other charges
        int dx = grid->xDistancei(newSite, charges[i]->site());
        int dy = grid->yDistancei(newSite, charges[i]->site());
        int dz = grid->zDistancei(newSite, charges[i]->site());
        if( dx < m_world->parameters()->electrostaticCutoff &&
            dy < m_world->parameters()->electrostaticCutoff &&
            dz < m_world->parameters()->electrostaticCutoff)
          {
            potential +=
              m_world->interactionEnergies()[dx][dy][dz] * charges[i]->charge();
              //count += 1;
          }
      }
    //qDebug() << QString("%1 %2 %3").arg(count).arg(chargeSize).arg(-1.0 * m_world->parameters()->electrostaticPrefactor*potential);
    return( -1.0 * potential );
  }

  inline double SourceAgent::imageInteraction(int newSite)
  {
    // Pointer to grid
    Grid *grid = m_world->electronGrid();

    // Reference to charges in simulation
    QList < ChargeAgent * >&charges = *m_world->electrons();

    // Number of charges in simulation
    int chargeSize = charges.size();

    // Coulomb Potential
    double potential = 0.0;

    //int count = 0;

    for(int i = 0; i < chargeSize; ++i)
      {
        // Potential at proposed site from other charges
        int dx = grid->xImageDistancei(newSite, charges[i]->site());
        int dy = grid->yDistancei(newSite, charges[i]->site());
        int dz = grid->zDistancei(newSite, charges[i]->site());
        if( dx < m_world->parameters()->electrostaticCutoff &&
            dy < m_world->parameters()->electrostaticCutoff &&
            dz < m_world->parameters()->electrostaticCutoff)
          {
            potential +=
              -1.0 * m_world->interactionEnergies()[dx][dy][dz] * charges[i]->charge();
            //count += 1;
          }
      }
    //qDebug() << QString("%1 %2 %3").arg(count).arg(chargeSize).arg(-1.0 * m_world->parameters()->electrostaticPrefactor*potential);
    return( -1.0 * potential );
  }

  inline double SourceAgent::siteInteraction(int newSite)
  {
    //qDebug() << QString("%1").arg( -1.0 * m_world->parameters()->elementaryCharge * m_world->grid()->potential(newSite) );
    return( -1.0 * m_world->electronGrid()->potential(newSite) );
  }

  inline bool SourceAgent::attemptTransport(double pd)
  {
    if(pd > 0)
      {
        if(exp(-pd*m_world->parameters()->inverseKT) > m_world->randomNumberGenerator()->random())
          {
           //qDebug() << 1;
           return true;
          }
        else
          {
           //qDebug() << 2;
           return false;
          }
      }
    else
      {
        //qDebug() << 3;
        return true;
      }
    //qDebug() << 4;
    return false;
  }
  int SourceAgent::transport()
  {
    // Do not inject if we are under the maximum charges
    if(m_charges >= m_maxCharges)
      {
        //qDebug() << "-";
        //qDebug() << QString("- %1 -").arg( m_world->charges()->size() );
        //qDebug() << QString("- %1 -").arg( m_world->charges()->size() );
        //qDebug() << 0;
        return -1;
      }

    // Select a random injection site
    int irn = m_world->randomNumberGenerator()->integer(0,m_neighbors.size()-1);

    int tries = 1;
    while(m_world->electronGrid()->agentAddress(m_neighbors[irn]))
      {
        irn = m_world->randomNumberGenerator()->integer(0,m_neighbors.size()-1);
        tries += 1;
        if(tries >= m_neighbors.size())
          return -1;
      }

    switch(m_world->parameters()->sourceType)
      {

      case 0:                        //constant case
        {
          //siteInteraction(m_neighbors[irn]);
          //coulombInteraction(m_neighbors[irn]);
          //imageInteraction(m_neighbors[irn]);
          // Do not inject 100*sourceBarrier percent of the time
          if(m_world->randomNumberGenerator()->random() <= m_world->parameters()->sourceBarrier)
            {
              //qDebug() << 5;
              return -1;
            }
          // Inject a charge carrier
          else
            {
              //qDebug() << 6;
              ++m_charges;
              return m_neighbors[irn];
            }
          break;
        }

      case 1:                        //coulomb loop case
        {
          double potential = siteInteraction(m_neighbors[irn]) + coulombInteraction(m_neighbors[irn]);
          //imageInteraction(m_neighbors[irn]);
          if ( attemptTransport(potential) )
            {
              ++m_charges;
              return m_neighbors[irn];
            }
          return -1;
          break;
        }

      case 2:                        //image charge case
        {
          double potential = siteInteraction(m_neighbors[irn]) + coulombInteraction(m_neighbors[irn]) + imageInteraction(m_neighbors[irn]);
          if ( attemptTransport(potential) )
            {
              ++m_charges;
              return m_neighbors[irn];
            }
          return -1;
          break;
        }

      default:                        //unknown case
        {
          qFatal("unknown injection probability calculation type encountered");
          return -1;
          break;
        }
      }
  }
  */
}
