#include "sourceagent.h"
#include "chargeagent.h"
#include "potential.h"
#include "inputparser.h"
#include "world.h"
#include "grid.h"
#include "rand.h"

namespace Langmuir
{

int SourceAgent::m_maxHoles     = 0;
int SourceAgent::m_maxElectrons = 0;

SourceAgent::SourceAgent(World * world) : Agent(Agent::Empty,world)
{
  m_maxHoles     = m_world->parameters()->chargePercentage*double(m_world->electronGrid()->volume());
  m_maxElectrons = m_world->parameters()->chargePercentage*double(m_world->electronGrid()->volume());
  m_attempts     = 0;
  m_successes    = 0;
  m_site         = 0;
  m_fSite        = 0;
  m_neighbors.clear();
  if ( m_world->electronGrid() )
  {
      Grid &grid = *m_world->electronGrid();
      m_neighborsL = grid.sliceIndex(0,1,0,grid.height(),0,grid.depth());
      m_neighborsR = grid.sliceIndex(grid.width()-1,grid.width(),0,grid.height(),0,grid.depth());
  }
  else
  {
      qFatal("SourceAgent allocation requires the Grid to exist");
  }
}

SourceAgent::~SourceAgent()
{
}

int SourceAgent::maxCarriers() const
{
    return maxElectrons() + maxHoles();
}

int SourceAgent::maxElectrons() const
{
    return m_maxElectrons;
}

int SourceAgent::maxHoles() const
{
    return m_maxHoles;
}

int SourceAgent::carrierCount() const
{
    return m_world->electrons()->size() + m_world->holes()->size();
}

int SourceAgent::electronCount() const
{
    return m_world->electrons()->size();
}

int SourceAgent::holeCount() const
{
    return m_world->holes()->size();
}

int SourceAgent::randomSiteID()
{
    return m_world->randomNumberGenerator()->integer(0,m_world->electronGrid()->volume()-1);
}

int SourceAgent::randomNeighborSiteID()
{
    return m_neighbors[m_world->randomNumberGenerator()->integer(0,m_neighbors.size()-1)];
}

int SourceAgent::randomLeftSiteID()
{
    return m_neighbors[m_world->randomNumberGenerator()->integer(0,m_neighborsL.size()-1)];
}

int SourceAgent::randomRightSiteID()
{
    return m_neighbors[m_world->randomNumberGenerator()->integer(0,m_neighborsR.size()-1)];
}

bool SourceAgent::tryRandom()
{
    m_attempts += 1;
    int site = randomNeighborSiteID();
    if ( validToInject(site) )
    {
        inject(site);
        m_successes += 1;
        return true;
    }
    return false;
}

bool SourceAgent::tryLeft()
{
    m_attempts += 1;
    int site = randomLeftSiteID();
    if ( validToInject(site) )
    {
        inject(site);
        m_successes += 1;
        return true;
    }
    return false;
}

bool SourceAgent::tryRight()
{
    m_attempts += 1;
    int site = randomRightSiteID();
    if ( validToInject(site) )
    {
        inject(site);
        m_successes += 1;
        return true;
    }
    return false;
}

ElectronSourceAgent::ElectronSourceAgent(World *world) : SourceAgent(world)
{
    m_type = Agent::ElectronSource;
    Grid &EGrid = *m_world->electronGrid();
    Grid &HGrid = *m_world->holeGrid();
    m_site  = EGrid.getIndexElectronSource();
    m_fSite = m_site;
    EGrid.setAgentAddress(m_site,this);
    EGrid.setAgentType(m_site,Agent::ElectronSource);
    HGrid.setAgentAddress(m_site,this);
    HGrid.setAgentType(m_site,Agent::ElectronSource);
}

HoleSourceAgent::HoleSourceAgent(World *world) : SourceAgent(world)
{
    m_type = Agent::HoleSource;
    Grid &EGrid = *m_world->electronGrid();
    Grid &HGrid = *m_world->holeGrid();
    m_site  = EGrid.getIndexHoleSource();
    m_fSite = m_site;
    EGrid.setAgentAddress(m_site,this);
    EGrid.setAgentType(m_site,Agent::HoleSource);
    HGrid.setAgentAddress(m_site,this);
    HGrid.setAgentType(m_site,Agent::HoleSource);
}

ExcitonSourceAgent::ExcitonSourceAgent(World *world) : SourceAgent(world)
{
    m_type = Agent::ExcitonSource;
    Grid &EGrid = *m_world->electronGrid();
    Grid &HGrid = *m_world->holeGrid();
    m_site  = EGrid.getIndexExcitonSource();
    m_fSite = m_site;
    EGrid.setAgentAddress(m_site,this);
    EGrid.setAgentType(m_site,Agent::ExcitonSource);
    HGrid.setAgentAddress(m_site,this);
    HGrid.setAgentType(m_site,Agent::ExcitonSource);
}

void ElectronSourceAgent::inject(int site)
{
    ChargeAgent *electron = new ChargeAgent(Agent::Electron,m_world,site);
    m_world->electrons()->push_back(electron);
}

void HoleSourceAgent::inject(int site)
{
    ChargeAgent *hole = new ChargeAgent(Agent::Hole,m_world,site);
    m_world->holes()->push_back(hole);
}

void ExcitonSourceAgent::inject(int site)
{
    ChargeAgent *electron = new ChargeAgent(Agent::Electron,m_world,site);
    m_world->electrons()->push_back(electron);

    ChargeAgent *hole = new ChargeAgent(Agent::Hole,m_world,site);
    m_world->holes()->push_back(hole);
}

bool ElectronSourceAgent::validToInject(int site)
{
    if ( electronCount() >= maxElectrons() ||
         //m_world->coupling()[m_type][Agent::Electron] <= 0 ||
         m_world->electronGrid()->agentType(site) != Agent::Empty ||
         m_world->electronGrid()->agentAddress(site) != 0 ||
         site >= m_world->electronGrid()->volume() ||
         m_world->defectSiteIDs()->contains(site) )
    {
        return false;
    }
    return true;
}

bool HoleSourceAgent::validToInject(int site)
{
    if ( holeCount() >= maxHoles() ||
         //m_world->coupling()[m_type][Agent::Hole] <= 0 ||
         m_world->holeGrid()->agentType(site) != Agent::Empty ||
         m_world->holeGrid()->agentAddress(site) != 0 ||
         site >= m_world->holeGrid()->volume() ||
         m_world->defectSiteIDs()->contains(site) )
    {
        return false;
    }
    return true;
}

bool ExcitonSourceAgent::validToInject(int site)
{
    if ( electronCount() >= maxElectrons() ||
         holeCount() >= maxHoles() ||

         //m_world->coupling()[m_type][Agent::Electron] <= 0 ||
         //m_world->coupling()[m_type][Agent::Hole] <= 0 ||

         m_world->electronGrid()->agentType(site) != Agent::Empty ||
         m_world->holeGrid()->agentType(site) != Agent::Empty ||

         m_world->electronGrid()->agentAddress(site) != 0 ||
         m_world->holeGrid()->agentAddress(site) != 0 ||

         site >= m_world->electronGrid()->volume() ||
         m_world->defectSiteIDs()->contains(site) )
    {
        return false;
    }
    return true;
}

/*

  bool SourceAgent::shouldInjectHole(int site)
  {
      return shouldInjectHoleUsingCoupling();
  }

  bool SourceAgent::shouldInjectElectron(int site)
  {
      return shouldInjectElectronUsingCoupling();
  }

  bool SourceAgent::shouldInjectHoleUsingCoupling()
  {
      return m_world->randomNumberGenerator()->randomlyChooseYesWithPercent(m_world->coupling()[m_type][Agent::Hole]);
  }

  bool SourceAgent::shouldInjectElectronUsingCoupling()
  {
      return m_world->randomNumberGenerator()->randomlyChooseYesWithPercent(m_world->coupling()[m_type][Agent::Electron]);
  }

  bool SourceAgent::shouldInjectHoleUsingMetropolis(int site)
  {
      double p1 = m_world->holeGrid()->potential(m_site);
      double p2 = m_world->holeGrid()->potential(site);
      double dE = p2-p1;
      return m_world->randomNumberGenerator()->randomlyChooseYesWithMetropolisAndCoupling(
              dE,m_world->parameters()->inverseKT,m_world->coupling()[Agent::Hole][m_type] );
  }

  bool SourceAgent::shouldInjectElectronUsingMetropolis(int site)
  {
      double p1 = m_world->electronGrid()->potential(m_site);
      double p2 = m_world->electronGrid()->potential(site);
      double dE = -1*(p2-p1);
      return m_world->randomNumberGenerator()->randomlyChooseYesWithMetropolisAndCoupling(
              dE,m_world->parameters()->inverseKT,m_world->coupling()[Agent::Electron][m_type] );
  }

  int SourceAgent::randomNeighborSiteID()
  {
      return m_neighbors[m_world->randomNumberGenerator()->integer(0,m_neighbors.size()-1)];
  }

  int SourceAgent::randomSiteID()
  {
      return m_world->randomNumberGenerator()->integer(0,m_world->electronGrid()->volume()-1);
  }

  bool SourceAgent::tryToPlaceHoleAtRandomSite()
  {
      int site = randomSiteID();
      if ( validToInjectHoleAtSite(site) )
      {
          injectHole(site);
          return true;
      }
      return false;
  }

  bool SourceAgent::tryToPlaceElectronAtRandomSite()
  {
      int site = randomSiteID();
      if ( validToInjectElectronAtSite(site) )
      {
          injectElectron(site);
          return true;
      }
      return false;
  }

  bool SourceAgent::tryToPlaceHoleAtNeighbor()
  {
      int site = randomNeighborSiteID();
      if ( validToInjectHoleAtSite(site) && shouldInjectHole(site) )
      {
          injectHole(site);
          return true;
      }
      return false;
  }

  bool SourceAgent::tryToPlaceElectronAtNeighbor()
  {
      int site = randomNeighborSiteID();
      if ( validToInjectElectronAtSite(site) && shouldInjectElectron(site) )
      {
          injectElectron(site);
          return true;
      }
      return false;
  }

  bool SourceAgent::tryToPlaceExcitonAtRandomSite()
  {
      int site = randomSiteID();
      if ( validToInjectHoleAtSite(site) && validToInjectElectronAtSite(site) )
      {
          injectElectron(site);
          injectHole(site);
          return true;
      }
      return false;
  }
  */
}
