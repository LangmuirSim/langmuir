#ifndef CHARGEAGENT_H
#define CHARGEAGENT_H

#include "agent.h"

#include <vector>

namespace Langmuir
{

  class ChargeAgent : public Agent
  {

  public:
    ChargeAgent(World *world, unsigned int site, bool coulombInteraction = true, double temperatureKelvin = 300.0, int zDefect = -1);
    virtual ~ChargeAgent();

    /**
     * Enable or disable Coulomb interaction for this charge carrier.
     */
    void setCoulombInteraction(bool enabled);
	  
	// Set the simulation temperature
	  
	double setTemperature(double temperatureKelvin);
	  
    // Enable or disable charged defects for this charge carrier
	
	void setChargedDefects(bool on);
	  
	//Set the charge on each defect
	  
	void setZdefect(int zDefect);

    /**
     * Returns the charge of this node.
     */
    virtual int charge();

    /**
     * Perform a transport attempt
     */
    virtual unsigned int transport();

    /**
     * Move on to the next time step.
     */
    virtual void completeTick();

    /**
     * Has the charge been removed from the system?
     */
    bool removed();

  protected:
    int m_charge;
	int m_zDefect;
	std::vector<unsigned int> m_neighbors;
    bool m_removed;
    bool m_coulombInteraction; // Should the Coulomb interaction be calculated?
	bool m_chargedDefects; // include charged defects?
	double m_temperatureKelvin;
    /**
     * Calculate the potential difference arising from the Coulomb interaction
     * between the two proposed sites.
     */
    double coulombInteraction(unsigned int newSite);
	  
    //Calculate the potential difference arising from elctrostatic interactions between
    //the carrier and a charged defect. 
	
    double defectsCharged(unsigned int newSite);
	  
    /**
     * Get the coupling constant for the proposed move.
     */
    double couplingConstant(short id1, short id2);

    /**
     * Was the transport attempt successful.
     */
    bool attemptTransport(double pd, double coupling, double T);
  };
	
 

  inline int ChargeAgent::charge()
  {
    return m_charge;
  }

  inline bool ChargeAgent::removed()
  {
    return m_removed;
  }

} // End namespace Langmuir

#endif // CHARGEAGENT_H
