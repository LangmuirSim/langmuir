#include "sourceagent.h"
#include "chargeagent.h"
#include "world.h"
#include "grid.h"
#include "rand.h"
#include "inputparser.h"

namespace Langmuir
{
  SourceAgent::SourceAgent(World * world, int site) : 
    Agent(Agent::Source, world,site)
  {
    m_charges = 0;
    m_site = m_world->grid()->volume();
    m_world->grid()->setAgent(m_world->grid()->volume(),this);
    m_world->grid()->setSiteID(m_world->grid()->volume(),2);
    m_maxCharges = m_world->parameters()->chargePercentage * double(m_world->grid()->volume());
  }

  SourceAgent::~SourceAgent()
  {
  }

  inline double SourceAgent::coulombInteraction(int newSite)
  {
    // Pointer to grid
    Grid *grid = m_world->grid();

    // Reference to charges in simulation
    QList < ChargeAgent * >&charges = *m_world->charges();

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
    return( -1.0 * m_world->parameters()->electrostaticPrefactor * potential );
  }

  inline double SourceAgent::imageInteraction(int newSite)
  {
    // Pointer to grid
    Grid *grid = m_world->grid();

    // Reference to charges in simulation
    QList < ChargeAgent * >&charges = *m_world->charges();

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
    return( -1.0 * m_world->parameters()->electrostaticPrefactor * potential );
  }

  inline double SourceAgent::siteInteraction(int newSite)
  {
    //qDebug() << QString("%1").arg( -1.0 * m_world->parameters()->elementaryCharge * m_world->grid()->potential(newSite) );
    return( -1.0 * m_world->parameters()->elementaryCharge * m_world->grid()->potential(newSite) );
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
    while(m_world->grid()->agent(m_neighbors[irn]))
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
}
