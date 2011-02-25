#include "sourceagent.h"
#include "chargeagent.h"
#include "world.h"
#include "grid.h"
#include "inputparser.h"

namespace Langmuir
{
  SourceAgent::SourceAgent ( World * world, unsigned int site ) : Agent( Agent::Source, world, site )
  {
   m_site = site;
   m_charges = 0;
   m_maxCharges = 100;
  }

  SourceAgent::~SourceAgent ()
  {
  }

  unsigned int SourceAgent::transport ()
  {

    // Determine whether a transport event will be attempted this tick
    // Used to maintain the average number of charges in the system
    if (m_charges >= m_maxCharges) 
    {
     //qDebug() << "R (we are at max charges)";
     return -1;
    }

    switch ( m_world->parameters()->sourceType )
    {
      case 0: //constant case
      {
       // Do not inject 100*m_pBarrier percent of the time
       if ( m_world->random() <= m_world->parameters()->sourceBarrier ) return -1;

       // Select a random neighbor
       int irn = int ( m_world->random () * double ( m_neighbors.size () - 0.00000001 ) );
       int tries = 1;
       while (m_world->grid ()->agent (m_neighbors[irn]))
       {
        irn = int ( m_world->random () * double ( m_neighbors.size () - 0.00000001 ) );
        tries += 1;
        if ( tries >= 1000 ) return -1;
       }

       // Inject a carrier
       ++m_charges;
       return m_neighbors[irn];

       break;
      }

      case 1: //coulomb loop case
      {
       // Select a random neighbor
       int irn = int ( m_world->random () * double ( m_neighbors.size () - 0.00000001 ) );
       int tries = 1;
       while (m_world->grid ()->agent (m_neighbors[irn]))
       {
        irn = int ( m_world->random () * double ( m_neighbors.size () - 0.00000001 ) );
        tries += 1;
        if ( tries >= 1000 ) 
        {
         //qDebug() << "R (we can't find an empty site)";
         return -1;
        }
       }

       double potential = 0;
       const double q4pe = 1.60217646e-19 / (4.0*M_PI*8.854187817e-12 * 3.5 * 1e-9);
       int cutoff = 50;
       Grid *grid = m_world->grid();
       const double kTinv = 1/(1.3806504e-23*m_world->parameters()->temperatureKelvin);
       {
        QList<ChargeAgent *> &charges = *m_world->charges();
        double v = 0.0;
        for (int i = 0; i < charges.size(); ++i) {
          int dx = grid->xDistancei(irn, charges[i]->site());
          int dy = grid->yDistancei(irn, charges[i]->site());
          int dz = grid->zDistancei(irn, charges[i]->site());
          if ( dx < cutoff && dy < cutoff && dz < cutoff ) { v += m_world->interactionEnergies()(dx,dy,dz)*charges[i]->charge(); }
        }
        potential += ( -1 * q4pe * v );
       }
/*
       if ( m_world->parameters()->chargedDefects )
       {
        QList<unsigned int> &chargedDefects = *m_world->chargedDefects();
        double v = 0.0;
        for (int i = 0; i < chargedDefects.size(); ++i) {
          int dx = grid->xDistancei(irn, chargedDefects[i]);
          int dy = grid->yDistancei(irn, chargedDefects[i]);
          int dz = grid->zDistancei(irn, chargedDefects[i]);
          if ( dx < cutoff && dy < cutoff && dz < cutoff ) { v += m_world->interactionEnergies()(dx,dy,dz); }
        }
        potential += m_world->parameters()->zDefect * q4pe * v;
       }

       if ( m_world->parameters()->chargedTraps )
       {
        QList<unsigned int> &chargedTraps = *m_world->chargedTraps();
        double v = 0.0;
        for (int i = 0; i < chargedDefects.size(); ++i) {
          int dx = grid->xDistancei(irn, chargedTraps[i]);
          int dy = grid->yDistancei(irn, chargedTraps[i]);
          int dz = grid->zDistancei(irn, chargedTraps[i]);
          if ( dx < cutoff && dy < cutoff && dz < cutoff ) { v += m_world->interactionEnergies()(dx,dy,dz); }
        }
        potential += m_world->parameters()->zTrap * q4pe * v;
       }
*/
       if ( potential > 0 )
       {
        double random_number = m_world->random();
        if ( exp(-potential*kTinv) > random_number )
        {
         ++m_charges; //qDebug() << "(A) V>0 ["<<potential<<"] E["<<exp(-potential*kTinv)<<"] R["<<random_number<<"]";
         return m_neighbors[irn];
        }
        else
        {
         //qDebug() << "(R) V>0["<<potential<<"] E["<<exp(-potential*kTinv)<<"] R["<<random_number<<"]";
         return -1;
        }
       }
       else
       {
        ++m_charges; //qDebug() << "(A) V<0["<<potential<<"] E["<<exp(-potential*kTinv)<<"] R["<<1.0<<"]";
        return m_neighbors[irn];
       }
       break;
      }

      default: //unknown case
      {
       qFatal("unknown injection probability calculation type encountered");
       break;
      }
    }

    return -1;
/*
    // Determine whether a transport event will be attempted this tick
    // Used to maintain the average number of charges in the system
    if (m_charges >= m_maxCharges)
      return -1;

    // Generate a random number
    double rn = m_world->random ();
    if (rn <= m_pBarrier)
      return -1;

    // Select a random neighbor and attempt transport.
    int irn =
      int (m_world->random () * double (m_neighbors.size () - 0.00000001));

    // Use another random number to determine whether the charge is injected
    if (!m_world->grid ()->agent (m_neighbors[irn]))
      {
        ++m_charges;:
        return m_neighbors[irn];
      }

    return -1;
*/
  }
}                                // End Langmuir namespace
