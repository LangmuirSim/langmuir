#include "chargeagent.h"
#include "world.h"
#include "grid.h" 
#include <QtCore/QThread>
#include <QtCore/QDebug>

#include <cstdlib>

namespace Langmuir{

  using   std::vector;
  using Eigen::Vector3d;

  ChargeAgent::ChargeAgent(World *world, unsigned int site, bool coulombInteraction, bool chargedDefects, bool chargedTraps , double temperatureKelvin, int zDefect, int zTrap) : 
   Agent(Agent::Charge, world, site), 
   m_charge(-1), 
   m_zDefect(zDefect), 
   m_zTrap(zTrap),
   m_removed(false),
   m_coulombInteraction(coulombInteraction), 
   m_chargedDefects(chargedDefects), 
   m_chargedTraps(chargedTraps),
   m_temperatureKelvin(temperatureKelvin)
  {
    //current site and future site are the same
    m_site = m_fSite;
    
    //make sure this agent is somewhere on the grid
    m_world->grid()->setAgent(m_site, this);

    //figure out the neighbors of this site
    m_neighbors = m_world->grid()->neighbors(m_site);
  }

  ChargeAgent::~ChargeAgent()
  {
  }

  void ChargeAgent::setCoulombInteraction(bool enabled)
  {
    m_coulombInteraction = enabled;
  }
	
  void ChargeAgent::setChargedDefects(bool on)
  {
    m_chargedDefects = on;
  }
	
  void ChargeAgent::setChargedTraps(bool ok)
  {
    m_chargedTraps = ok;
  }

  unsigned int ChargeAgent::transport()
  {
   // Magnitude of charge on an electron
   const double q = 1.60217646e-19; // Magnitude of charge on an electron

   // Get a pointer to the grid for easy access
   Grid *grid = m_world->grid();

   // Select a proposed transport site at random, but esure that it is not the source
	  unsigned int newSite;
	  do{newSite = m_neighbors[int(m_world->random()*(m_neighbors.size()-1.0e-20))];}
	  while (grid->siteID(newSite) == 2); // source siteID

   // Check if the proposed site to move to is already occupied, return -1 if unsucessful
   // Also return if we picked the drain
   if (grid->agent(newSite) && grid->siteID(newSite) != 3) return -1;

   // Now to add on the background potential - from the applied field
   // A few electrons only perturb the potential by ~1e-20 or so.
   double pd = grid->potential(newSite) - grid->potential(m_site);
   pd *= m_charge * q;

   // Add on the Coulomb interaction if it is being included
   if (m_coulombInteraction)
    pd += this->coulombInteraction(newSite);
	  
   // Add the interactions from charged defects
   if (m_chargedDefects) pd += defectsCharged(newSite);
	  
   // Add the interactions from charged traps
   if (m_chargedTraps) pd += trapsCharged(newSite);
	  
   // Get the coupling constant
   double coupling = couplingConstant(grid->siteID(m_site),grid->siteID(newSite));

   // Get the temperature
   double T = m_temperatureKelvin;

   // Apply metropolis criterion	  
   if (attemptTransport(pd, coupling, T)) {
    // Set the future site is sucessful
    m_fSite = newSite;
    return m_fSite;
   }

   // Return -1 if transport unsucessful
   return -1;
  }

  void ChargeAgent::completeTick()
  {
    if (m_site != m_fSite) {

     // Are we on a drain site? If so then we have been removed.
     // Apparently charge carriers are people too.
     if ( m_world->grid()->siteID(m_fSite) == 3 ) {
      m_world->grid()->setAgent(m_site, 0);
      m_removed = true;
      return;
     }

     // Check if another charge snuck into this site before us.
     else if ( m_world->grid()->agent(m_fSite) ) {
     // Abort the move - site is now occupied
     m_fSite = m_site;
     return;
     }

     if (m_site != errorValue)
      // Vacate the old site
      m_world->grid()->setAgent(m_site, 0);

      // Everything looks good - set this agent to the new site.
      m_site = m_fSite;
      m_world->grid()->setAgent(m_site, this);

      // Update our neighbors
      m_neighbors = m_world->grid()->neighbors(m_site);
     }
  }

inline double ChargeAgent::coulombInteraction(unsigned int newSite)
{
 const double q = 1.60217646e-19;

 // Prefactor for force calculations q / 4 pi epsilon with a 1e-9 for m -> nm
 // Using a relative permittivity of 3.5 for now - should be configurable
 const double q4pe = q / (4.0*M_PI*8.854187817e-12 * 3.5 * 1e-9);
	
 // Cutting off interaction energies at 50nm
 int cutoff = 50;

 // Pointer to grid
 Grid *grid = m_world->grid();

 // Reference to charges in simulation
 QList<ChargeAgent *> &charges = *m_world->charges();

 // Number of charges in simulation
 int chargeSize = charges.size();

 // Potential before
 double potential1 = 0.0;
  
 // Potential after
 double potential2 = 0.0;

 for (int i = 0; i < chargeSize; ++i) {

  if (charges[i] != this) {

   // Potential at current site from other charges
   int dx = grid->xDistancei(m_site, charges[i]->site());
   int dy = grid->yDistancei(m_site, charges[i]->site());
   int dz = grid->zDistancei(m_site, charges[i]->site());

   if ( dx < cutoff && dy < cutoff && dz < cutoff ) {
    potential1 += m_world->interactionEnergies()(dx,dy,dz)*charges[i]->charge();
   }

   // Potential at new site from other charges
   if (newSite != charges[i]->site()) {
    dx = grid->xDistancei(newSite, charges[i]->site());
    dy = grid->yDistancei(newSite, charges[i]->site());
    dz = grid->zDistancei(newSite, charges[i]->site());
    if ( dx < cutoff && dy < cutoff && dz < cutoff ) {
     potential2 += m_world->interactionEnergies()(dx,dy,dz)*charges[i]->charge();
    }
   }
   else {
    return -1;
   }
  }
 }
 return m_charge * q4pe * (potential2 - potential1);
}

inline double ChargeAgent::defectsCharged(unsigned int newSite)
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
 return m_charge * m_zDefect * q4pe * (potential2 - potential1);
}

	inline double ChargeAgent::trapsCharged(unsigned int newSite)
	{
		const double q = 1.60217646e-19; // Magnitude of charge on an electron
		// Prefactor for force calculations q / 4 pi epsilon with a 1e-9 for m -> nm
		const double q4pe = q / (4.0*M_PI*8.854187817e-12 * 3.5 * 1e-9);
		
		// Cutting off interaction energies at 50nm
		int cutoff = 50;
		
		Grid *grid = m_world->grid();
		
		// Figure out the potential of the site we are on
		QList<unsigned int> &chargedTraps = *m_world->chargedTraps();
		double potential1(0.0), potential2(0.0);
		int trapSize(chargedTraps.size());
		for (int i = 0; i < trapSize; ++i) {
			// Potential at current site from charged traps
			int dx = grid->xDistancei(m_site, chargedTraps[i]);
			int dy = grid->yDistancei(m_site, chargedTraps[i]);
			int dz = grid->zDistancei(m_site, chargedTraps[i]);
			if (dx < cutoff && dy < cutoff && dz < cutoff && dx != 0 && dy != 0 && dz != 0) {
				potential1 += m_world->interactionEnergies()(dx,dy,dz)*chargedTraps[i];
			}
			// Potential at new site from charged traps
			if (newSite != chargedTraps[i]) {
				dx = grid->xDistancei(newSite, chargedTraps[i]);
				dy = grid->yDistancei(newSite, chargedTraps[i]);
				dz = grid->zDistancei(newSite, chargedTraps[i]);
				if (dx < cutoff && dy < cutoff && dx != 0 && dy != 0 && dz != 0) {
					potential2 += m_world->interactionEnergies()(dx,dy,dz)*chargedTraps[i];
				}
			}
			else {
				return -1;
			}
		}
		return m_charge * m_zTrap * q4pe * (potential2 - potential1);
	}
	
inline double ChargeAgent::couplingConstant(short id1, short id2)
{
 return (*m_world->coupling())(id1, id2);
}

inline bool ChargeAgent::attemptTransport(double pd, double coupling, double T)
{
 // Now to decide if the hop will be made - using a simple exponential energy
 // change acceptance now.

 //Define the Boltzmann constant
 const double k = 1.3806504e-23;
 const double kTinv = 1 / (k * T); 
 double randNumber = m_world->random();
//    qDebug() << "Deciding on whether to accept move..." << pd << exp(-pd * kTinv)
//        << randNumber;
 if (pd > 0.0) {
//      qDebug() << "Method 1:" << pd << exp(-pd * kTinv) << randNumber;
 if ((coupling * exp(-pd * kTinv)) > randNumber) {
//        qDebug() << "Charge move accepted..." << m_fSite;
 return true;
 }
}
else if (coupling > randNumber) {
//      qDebug() << "Method 2:" << pd << randNumber;
//      qDebug() << "Charge move accepted (2)..." << m_fSite;
return true;
}
return false;
}
}
