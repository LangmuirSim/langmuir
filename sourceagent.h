#ifndef SOURCEAGENT_H
#define SOURCEAGENT_H

#include "agent.h"

namespace Langmuir
{
  class SourceAgent:public Agent
  {
  public:

    SourceAgent(World * world);
   ~SourceAgent();

    virtual int maxCarriers() const;
    virtual int maxElectrons() const;
    virtual int maxHoles() const;
    virtual int carrierCount() const;
    virtual int electronCount() const;
    virtual int holeCount() const;
    virtual double percentCarriers() const;
    virtual double percentHoles() const;
    virtual double percentElectrons() const;
    virtual unsigned long int attempts() const;
    virtual unsigned long int successes() const;
    virtual double successRate() const;
    virtual void resetCounters();
    virtual bool seed();
    virtual bool tryToInject();
    virtual QString toQString() const;

  protected:

    virtual void inject(int site) = 0;
    virtual bool validToInject(int site);

    virtual int randomSiteID();
    virtual int randomNeighborSiteID();

    virtual double energyChange(int site);
    virtual bool shouldInject(int site);

    static int m_maxHoles;
    static int m_maxElectrons;
    unsigned long int m_attempts;
    unsigned long int m_successes;
  };

  class ElectronSourceAgent : public SourceAgent
  {
  public:
      ElectronSourceAgent(World * world) : SourceAgent(world) {}
  protected:
      void inject(int site);
      bool validToInject(int site);
      double energyChange(int fSite);
  };

  class ElectronSourceAgentLeft : public ElectronSourceAgent
  {
  public:
      ElectronSourceAgentLeft(World * world);
  };

  class ElectronSourceAgentRight : public ElectronSourceAgent
  {
  public:
      ElectronSourceAgentRight(World * world);
  };

  class HoleSourceAgent : public SourceAgent
  {
  public:
      HoleSourceAgent(World * world) : SourceAgent(world) {}
  protected:
      void inject(int site);
      bool validToInject(int site);
      double energyChange(int fSite);
  };

  class HoleSourceAgentLeft : public HoleSourceAgent
  {
  public:
      HoleSourceAgentLeft(World * world);
  };

  class HoleSourceAgentRight : public HoleSourceAgent
  {
  public:
      HoleSourceAgentRight(World * world);
  };

  class ExcitonSourceAgent : public SourceAgent
  {
  public:
      ExcitonSourceAgent(World * world);
      virtual bool tryToInject();
  protected:
      void inject(int site);
      bool validToInject(int site);
  };
}
#endif
