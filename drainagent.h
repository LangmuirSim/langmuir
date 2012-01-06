#ifndef DRAINAGENT_H
#define DRAINAGENT_H

#include "fluxagent.h"

namespace Langmuir
{
class DrainAgent : public FluxAgent
{
public:
    DrainAgent(World * world, double potential = 0.0, double rate = 0.0, int tries = 1);
protected:
};

class ElectronDrainAgent : public DrainAgent
{
public:
    ElectronDrainAgent(World * world, int site, double potential = 0.0, double rate = 0.0, int tries = 1);
    ElectronDrainAgent(World * world, Grid::CubeFace cubeFace, double potential = 0.0, double rate = 0.0, int tries = 1);
protected:
    virtual double energyChange(int fSite);
};

class HoleDrainAgent : public DrainAgent
{
public:
    HoleDrainAgent(World * world, int site, double potential = 0.0, double rate = 0.0, int tries = 1);
    HoleDrainAgent(World * world, Grid::CubeFace cubeFace, double potential = 0.0, double rate = 0.0, int tries = 1);
protected:
    virtual double energyChange(int fSite);
};
}

/*
#include "agent.h"
#include "cubicgrid.h"

namespace Langmuir
{
  class ChargeAgent;

  class DrainAgent : public Agent
  {
  public:
    DrainAgent(World * world, double potential = 0.0, double rate = 0.0, int tries = 1);
   ~DrainAgent();
    virtual void acceptCharge(Agent::Type type);
    virtual bool tryToAccept(ChargeAgent *charge);
    unsigned long int attempts() const;
    unsigned long int successes() const;
    double successRate() const;
    void resetCounters();

  protected:
    virtual double energyChange(ChargeAgent *charge);
    virtual bool shouldAccept(ChargeAgent *charge);
    unsigned long int m_attempts;
    unsigned long int m_successes;
    double m_rate;
    int m_tries;
    double m_potential;
    Grid *m_grid;
    Grid::CubeFace m_face;
  };

  class ElectronDrainAgent : public DrainAgent
  {
  public:
      ElectronDrainAgent(World * world, int site, double potential = 0.0, double rate = 0.0, int tries = 1);
      ElectronDrainAgent(World * world, Grid::CubeFace cubeFace, double potential = 0.0, double rate = 0.0, int tries = 1);
  };

  class HoleDrainAgent : public DrainAgent
  {
  public:
      HoleDrainAgent(World * world, int site, double potential = 0.0, double rate = 0.0, int tries = 1);
      HoleDrainAgent(World * world, Grid::CubeFace cubeFace, double potential = 0.0, double rate = 0.0, int tries = 1);
  };
}
*/

#endif
