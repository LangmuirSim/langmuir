#ifndef DRAINAGENT_H
#define DRAINAGENT_H

#include "agent.h"

namespace Langmuir
{
  class ChargeAgent;

  class DrainAgent : public Agent
  {

  public:

    DrainAgent(World * world);
   ~DrainAgent();

    virtual void acceptCharge(Agent::Type type);
    virtual bool tryToAccept(ChargeAgent *charge);

    virtual unsigned long int acceptedCarriers() const;
    virtual unsigned long int acceptedElectrons() const;
    virtual unsigned long int acceptedHoles() const;
    virtual unsigned long int attempts() const;
    virtual unsigned long int successes() const;
    virtual double successRate() const;

    virtual QString toQString() const;

  private:

    virtual double energyChange(ChargeAgent *charge);
    virtual bool shouldAccept(ChargeAgent *charge);

    unsigned long int m_acceptedElectrons;
    unsigned long int m_acceptedHoles;
    unsigned long int m_attempts;
    unsigned long int m_successes;
  };

  class ElectronDrainAgent : public DrainAgent
  {
  public:
      ElectronDrainAgent(World *world) : DrainAgent(world) {}
  };

  class ElectronDrainAgentLeft : public ElectronDrainAgent
  {
  public:
      ElectronDrainAgentLeft(World *world);
  };

  class ElectronDrainAgentRight : public ElectronDrainAgent
  {
  public:
      ElectronDrainAgentRight(World *world);
  };

  class HoleDrainAgent : public DrainAgent
  {
  public:
      HoleDrainAgent(World *world) : DrainAgent(world) {}
  };

  class HoleDrainAgentLeft : public HoleDrainAgent
  {
  public:
      HoleDrainAgentLeft(World *world);
  };

  class HoleDrainAgentRight : public HoleDrainAgent
  {
  public:
      HoleDrainAgentRight(World *world);
  };
}

#endif
