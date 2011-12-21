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

    int maxCarriers() const;
    int maxElectrons() const;
    int maxHoles() const;
    int carrierCount() const;
    int electronCount() const;
    int holeCount() const;

    virtual bool tryRandom();
    virtual bool tryLeft();
    virtual bool tryRight();

  protected:

    virtual void inject(int site) = 0;
    virtual bool validToInject(int site) = 0;

    int randomSiteID();
    int randomNeighborSiteID();
    int randomRightSiteID();
    int randomLeftSiteID();

    static int m_maxHoles;
    static int m_maxElectrons;
    unsigned int m_attempts;
    unsigned int m_successes;

    QVector<int> m_neighborsL;
    QVector<int> m_neighborsR;
  };

  class ElectronSourceAgent : public SourceAgent
  {
  public:
      ElectronSourceAgent(World * world);
  protected:
      void inject(int site);
      bool validToInject(int site);
  };

  class HoleSourceAgent : public SourceAgent
  {
  public:
      HoleSourceAgent(World * world);
  protected:
      void inject(int site);
      bool validToInject(int site);
  };

  class ExcitonSourceAgent : public SourceAgent
  {
  public:
      ExcitonSourceAgent(World * world);
  protected:
      void inject(int site);
      bool validToInject(int site);
  };
}
/*
  class SourceAgent:public Agent
  {
  public:
    SourceAgent(Agent::Type type, World * world, int site);
    ~SourceAgent();

    int maxCarriers();
    int maxElectrons();
    int maxHoles();
    int carrierCount();
    int electronCount();
    int holeCount();

    bool tryToPlaceHoleAtRandomSite();
    bool tryToPlaceElectronAtRandomSite();
    bool tryToPlaceHoleAtNeighbor();
    bool tryToPlaceElectronAtNeighbor();
    bool tryToPlaceExcitonAtRandomSite();

  private:

    unsigned int m_attempts;
    unsigned int m_successes;
    static int m_maxElectrons;
    static int m_maxHoles;

    void injectHole(int site);
    void injectElectron(int site);

    bool shouldInjectHole(int site);
    bool shouldInjectElectron(int site);
    bool shouldInjectHoleUsingCoupling();
    bool shouldInjectElectronUsingCoupling();
    bool shouldInjectHoleUsingMetropolis(int site);
    bool shouldInjectElectronUsingMetropolis(int site);

    bool validToInjectHoleAtSite(int site);
    bool validToInjectElectronAtSite(int site);

    int randomSiteID();
    int randomNeighborSiteID();
  };
  */
#endif
