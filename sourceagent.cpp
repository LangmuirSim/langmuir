#include "sourceagent.h"
#include "chargeagent.h"
#include "potential.h"
#include "inputparser.h"
#include "logger.h"
#include "world.h"
#include "grid.h"
#include "rand.h"
#include "limits.h"

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
  m_neighbors = QVector<int>() << -1;
  if ( m_world->electronGrid() == 0 )
  {
      qFatal("SourceAgent allocation requires the Grid to exist");
  }
}

SourceAgent::~SourceAgent()
{
    qDebug() << *this << "\n";
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

bool SourceAgent::validToInject(int site)
{
    return false;
}

double SourceAgent::energyChange(int site)
{
    return 0;
}

double ElectronSourceAgent::energyChange(int site)
{
    double p1 = m_world->electronGrid()->potential(m_site);
    double p2 = m_world->electronGrid()->potential(site);
    return p1-p2; // its backwards because q=-1 and dE = q*(p2-p1) = p1-p2
}

double HoleSourceAgent::energyChange(int site)
{
    double p1 = m_world->holeGrid()->potential(m_site);
    double p2 = m_world->holeGrid()->potential(site);
    return p2-p1;
}

bool SourceAgent::shouldInject(int site)
{
    return m_world->randomNumberGenerator()->randomlyChooseYesWithPercent(m_world->coupling()[m_type][Agent::Empty]);
    //return m_world->randomNumberGenerator()->randomlyChooseYesWithMetropolisAndCoupling(
    //        energyChange(site),m_world->parameters()->inverseKT,m_world->coupling()[m_type][Agent::Empty] );
}

void SourceAgent::resetCounters()
{
    m_attempts = 0;
    m_successes = 0;
}

unsigned long int SourceAgent::attempts() const
{
    return m_attempts;
}

unsigned long int SourceAgent::successes() const
{
    return m_successes;
}

double SourceAgent::successRate() const
{
    if ( m_attempts > 0 )
    {
        return double(m_successes)/double(m_attempts)*100.0;
    }
    else
    {
        return 0.0;
    }
}

double SourceAgent::percentCarriers() const
{
    if ( maxCarriers() > 0 )
    {
        return double(carrierCount())/double(maxCarriers())*100.0;
    }
    else
    {
        return 0.0;
    }
}

double SourceAgent::percentHoles() const
{
    if ( maxHoles() > 0 )
    {
        return double(holeCount())/double(maxHoles())*100.0;
    }
    else
    {
        return 0.0;
    }
}

double SourceAgent::percentElectrons() const
{
    if ( maxElectrons() > 0 )
    {
        return double(electronCount())/double(maxElectrons())*100.0;
    }
    else
    {
        return 0.0;
    }
}

bool SourceAgent::seed()
{
    int site = randomSiteID();
    double coupling = m_world->coupling()[m_type][Agent::Empty];
    m_world->coupling()[m_type][Agent::Empty] = 1.0;
    if ( validToInject(site) )
    {
        inject(site);
        m_world->coupling()[m_type][Agent::Empty] = coupling;
        return true;
    }
    m_world->coupling()[m_type][Agent::Empty] = coupling;
    return false;
}

bool SourceAgent::tryToInject()
{
    m_attempts += 1;
    int site = randomNeighborSiteID();
    if ( validToInject(site) && shouldInject(site) )
    {
        inject(site);
        m_successes += 1;
        return true;
    }
    return false;
}

bool ExcitonSourceAgent::tryToInject()
{
    m_attempts += 1;
    int site = randomSiteID();
    if ( validToInject(site) && shouldInject(site) )
    {
        inject(site);
        m_successes += 1;
        return true;
    }
    return false;
}

ElectronSourceAgentLeft::ElectronSourceAgentLeft(World *world) : ElectronSourceAgent(world)
{
    m_type = Agent::ElectronSourceL;
    Grid &grid = *m_world->electronGrid();
    m_site  = grid.getIndexSourceL();
    m_fSite = m_site;
    grid.setAgentAddress(m_site,this);
    grid.setAgentType(m_site,Agent::ElectronSourceL);
    m_neighbors = grid.sliceIndex(0,1,0,grid.height(),0,grid.depth());
}

ElectronSourceAgentRight::ElectronSourceAgentRight(World *world) : ElectronSourceAgent(world)
{
    m_type = Agent::ElectronSourceR;
    Grid &grid = *m_world->electronGrid();
    m_site  = grid.getIndexSourceR();
    m_fSite = m_site;
    grid.setAgentAddress(m_site,this);
    grid.setAgentType(m_site,Agent::ElectronSourceR);
    m_neighbors = grid.sliceIndex(grid.width()-1,grid.width(),0,grid.height(),0,grid.depth());
}

HoleSourceAgentLeft::HoleSourceAgentLeft(World *world) : HoleSourceAgent(world)
{
    m_type = Agent::HoleSourceL;
    Grid &grid = *m_world->holeGrid();
    m_site  = grid.getIndexSourceL();
    m_fSite = m_site;
    grid.setAgentAddress(m_site,this);
    grid.setAgentType(m_site,Agent::HoleSourceL);
    m_neighbors = grid.sliceIndex(0,1,0,grid.height(),0,grid.depth());
}

HoleSourceAgentRight::HoleSourceAgentRight(World *world) : HoleSourceAgent(world)
{
    m_type = Agent::HoleSourceR;
    Grid &grid = *m_world->holeGrid();
    m_site  = grid.getIndexSourceR();
    m_fSite = m_site;
    grid.setAgentAddress(m_site,this);
    grid.setAgentType(m_site,Agent::HoleSourceR);
    m_neighbors = grid.sliceIndex(grid.width()-1,grid.width(),0,grid.height(),0,grid.depth());
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
    m_neighbors = QVector<int>() << -1;
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
         m_world->coupling()[m_type][Agent::Empty] <= 0 ||
         site < 0 ||
         site >= m_world->electronGrid()->volume() ||
         m_world->electronGrid()->agentType(site) != Agent::Empty ||
         m_world->electronGrid()->agentAddress(site) != 0 ||
         m_world->defectSiteIDs()->contains(site) )
    {
        return false;
    }
    return true;
}

bool HoleSourceAgent::validToInject(int site)
{
    if ( holeCount() >= maxHoles() ||
         m_world->coupling()[m_type][Agent::Empty] <= 0 ||
         site < 0 ||
         site >= m_world->holeGrid()->volume() ||
         m_world->holeGrid()->agentType(site) != Agent::Empty ||
         m_world->holeGrid()->agentAddress(site) != 0 ||
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
         m_world->coupling()[m_type][Agent::Empty] <= 0 ||
         site < 0 ||
         site >= m_world->electronGrid()->volume() ||
         m_world->electronGrid()->agentType(site) != Agent::Empty ||
         m_world->holeGrid()->agentType(site) != Agent::Empty ||
         m_world->electronGrid()->agentAddress(site) != 0 ||
         m_world->holeGrid()->agentAddress(site) != 0 ||
         m_world->defectSiteIDs()->contains(site) )
    {
        return false;
    }
    return true;
}

QString SourceAgent::toQString() const
{
    QString result = Agent::toQString();
    result = result.replace( QRegExp("\\s*\\]\\s*$"), "\n" );
    QTextStream stream(&result);

    int w = 20;
    int p =  2;

    stream << QString("%1 %2 %3 %4 %5")
              .arg(       "carriers:", w        )
              .arg(    carrierCount(), w        )
              .arg(     maxCarriers(), w        )
              .arg( percentCarriers(), w, 'f', p)
              .arg('\n');

    stream << QString("%1 %2 %3 %4 %5")
              .arg(      "electrons:", w        )
              .arg(   electronCount(), w        )
              .arg(    maxElectrons(), w        )
              .arg(percentElectrons(), w, 'f', p)
              .arg('\n');

    stream << QString("%1 %2 %3 %4 %5")
              .arg(          "holes:", w        )
              .arg(       holeCount(), w        )
              .arg(        maxHoles(), w        )
              .arg(    percentHoles(), w, 'f', p)
              .arg('\n');

    stream << QString("%1 %2 %3 %4 %5")
              .arg(           "rate:", w        )
              .arg(       successes(), w        )
              .arg(        attempts(), w        )
              .arg(     successRate(), w, 'f', p)
              .arg('\n');

    QStringList cpl = m_world->logger()->couplingStringList(m_type,Agent::Empty);
    stream << QString("%1 %2 %3 %4 %5")
              .arg(       "coupling:", w        )
              .arg(            cpl[0], w        )
              .arg(            cpl[1], w        )
              .arg(            cpl[2], w        )
              .arg(']',1);

    return result;
}

}
