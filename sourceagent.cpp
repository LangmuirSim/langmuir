#include "sourceagent.h"
#include "world.h"
#include "grid.h"

namespace Langmuir
{
  SourceAgent::SourceAgent ( World * world, unsigned int site, double potential, double barrier, int barrierCalculationType, 
   bool chargedDefects, bool chargedTraps, double temperatureKelvin, int zDefect, int zTrap ):
   Agent(Agent::Source, world, site)
  {
   m_site = site;
   m_potential = potential;
   m_pBarrier  = barrier;
   m_pBarrierCalculationType = barrierCalculationType;
   m_chargedDefects = chargedDefects;
   m_chargedTraps = chargedTraps;
   m_temperatureKelvin = temperatureKelvin;
   m_zDefect = zDefect;
   m_zTrap = zTrap;
   m_charges = 0;
   m_maxCharges = 100;

   qDebug() << "INPUT WORLD:        " << world;
   qDebug() << "INPUT SITE:         " << site;
   qDebug() << "INPUT POTENTIAL:    " << potential;
   qDebug() << "INPUT PBARRIER:     " << barrier;
   qDebug() << "INPUT CALC:         " << barrierCalculationType;
   qDebug() << "INPUT DEFECT:       " << chargedDefects;
   qDebug() << "INPUT TRAP:         " << chargedTraps;
   qDebug() << "INPUT TEMP:         " << temperatureKelvin;
   qDebug() << "INPUT ZDEFECT:      " << zDefect;
   qDebug() << "INPUT ZTRAP:        " << zTrap;

   qDebug() << "SOURCE WORLD:       " << m_world;
   qDebug() << "SOURCE SITE:        " << this->site();
   qDebug() << "SOURCE POTENTIAL:   " << m_potential;
   qDebug() << "SOURCE PBARRIER:    " << m_pBarrier;
   qDebug() << "SOURCE CALC:        " << m_pBarrierCalculationType;
   qDebug() << "SOURCE DEFECT:      " << m_chargedDefects;
   qDebug() << "SOURCE TRAP:        " << m_chargedTraps;
   qDebug() << "SOURCE TEMP:        " << m_temperatureKelvin;
   qDebug() << "SOURCE ZDEFECT:     " << m_zDefect;
   qDebug() << "SOURCE ZTRAP:       " << m_zTrap;
   qDebug() << "SOURCE CHARGES:     " << m_charges;
   qDebug() << "SOURCE MAXCHARGES:  " << m_maxCharges;
  qFatal("bye");
  }

  SourceAgent::~SourceAgent ()
  {
  }

  int SourceAgent::charge ()
  {
    // Always has a charge of 1
    return 1;
  }

  unsigned int SourceAgent::transport ()
  {

    // Determine whether a transport event will be attempted this tick
    // Used to maintain the average number of charges in the system
    if (m_charges >= m_maxCharges) return -1;

    switch ( m_pBarrierCalculationType )
    {
      case 0: //constant case
      {
       // Do not inject 100*m_pBarrier percent of the time
       if ( m_world->random() <= m_pBarrier ) return -1;

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
        if ( tries >= 1000 ) return -1;
       }

       // Perform a Coulomb calculation
       const double q = 1.60217646e-19; // Magnitude of charge on an electron
       // Prefactor for force calculations q / 4 pi epsilon with a 1e-9 for m -> nm
       const double q4pe = q / (4.0*M_PI*8.854187817e-12 * 3.5 * 1e-9);
       // Cutting off interaction energies at 50nm
       int cutoff = 50;
       // Pointer to the grid
       Grid *grid = m_world->grid();

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
 {
  const double q = 1.60217646e-19; // Magnitude of charge on an electron
  // Prefactor for force calculations q / 4 pi epsilon with a 1e-9 for m -> nm
  const double q4pe = q / (4.0*M_PI*8.854187817e-12 * 3.5 * 1e-9);
	
  // Cutting off interaction energies at 50nm
  int cutoff = 50;

  Grid *grid = m_world->grid();
		
  // Figure out the potential of the site we are on
  QList<unsigned int> &chargedDefects = *m_world->chargedDefects();
  double potential1(0.0), potential2(0.0);
  int defectSize(chargedDefects.size());
  for (int i = 0; i < defectSize; ++i) {
  // Potential at current site from charged defects
  int dx = grid->xDistancei(m_site, chargedDefects[i]);
  int dy = grid->yDistancei(m_site, chargedDefects[i]);
  int dz = grid->zDistancei(m_site, chargedDefects[i]);
  if (dx < cutoff && dy < cutoff && dz < cutoff) {
   potential1 += m_world->interactionEnergies()(dx,dy,dz)*chargedDefects[i];
  }
  // Potential at new site from charged defects
  if (newSite != chargedDefects[i]) {
   dx = grid->xDistancei(newSite, chargedDefects[i]);
   dy = grid->yDistancei(newSite, chargedDefects[i]);
   dz = grid->zDistancei(newSite, chargedDefects[i]);
  if (dx < cutoff && dy < cutoff && dz < cutoff) {
   potential2 += m_world->interactionEnergies()(dx,dy,dz)*chargedDefects[i];
  }
 }
 else {
 return -1;
 }
}
*/
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
