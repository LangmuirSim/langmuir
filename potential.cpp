#include "potential.h"
#include "simulation.h"
#include "chargeagent.h"
#include "inputparser.h"
#include "cubicgrid.h"
#include "world.h"
#include "rand.h"

using namespace Langmuir;

Potential::Potential( World* world ) : m_world( world ) {}

void Potential::setPotentialZero()
{
    for ( int i = 0; i < m_world->grid()->volume(); i++ )
    {
        m_world->grid()->setPotential( i, 0 );
    }
}

void Potential::setPotentialLinear()
{
    double  m = ( m_world->parameters()->voltageDrain - m_world->parameters()->voltageSource ) / double( m_world->grid()->width() );
    double  b = m_world->parameters()->voltageSource;
    for ( int i = 0; i < m_world->grid()->width(); i++ )
    {
        for ( int j = 0; j < m_world->grid()->height(); j++ )
        {
            for ( int k = 0; k < m_world->grid()->depth(); k++ )
            {
                int s = m_world->grid()->getIndex(i,j,k);
                double v = m * ( i + 0.5 ) + b;
                m_world->grid()->addToPotential(s,v);
            }
        }
    }
    m_world->grid()->setDrainPotential(m_world->parameters()->voltageDrain);
    m_world->grid()->setSourcePotential(m_world->parameters()->voltageSource);
}

void Potential::setPotentialTraps()
{
    // Do nothing if we shouldn't have traps
    if ( m_world->parameters()->trapPercentage <= 0 ) return;
    if ( m_world->parameters()->seedPercentage <= 0 ) return;

    // Seed traps, if seed.percentage=100% then this is just placing traps normally
    int progress = int( m_world->grid()->volume() * m_world->parameters()->trapPercentage * m_world->parameters()->seedPercentage );
    while ( progress >= m_world->trapSiteIDs()->size() )
    {
        int s = m_world->randomNumberGenerator()->integer(0,m_world->grid()->volume()-1);

        if ( ! m_world->trapSiteIDs()->contains(s) )
        {
            m_world->grid()->addToPotential(s,m_world->parameters()->deltaEpsilon);
            m_world->trapSiteIDs()->push_back(s);
        }
    }

    // Do not grow traps if we aren't growing hetergeneous traps.
    if ( m_world->parameters()->trapPercentage <= 0 ) return;
    if ( m_world->parameters()->seedPercentage <= 0 ||
         m_world->parameters()->seedPercentage == 1 ) return;
    progress = int( m_world->grid()->volume() * m_world->parameters()->trapPercentage );
    while ( progress >= m_world->trapSiteIDs()->size() )
    {
        int trapSeedIndex               = m_world->randomNumberGenerator()->integer(0,m_world->trapSiteIDs()->size()-1);
        int trapSeedSite                = m_world->trapSiteIDs()->at(trapSeedIndex);
        QVector<int> trapSeedNeighbors  = m_world->simulation()->neighborsSite(trapSeedSite);

        int newTrapIndex                = m_world->randomNumberGenerator()->integer(0,trapSeedNeighbors.size()-1);
        int newTrapSite                 = trapSeedNeighbors[newTrapIndex];
        if ( m_world->grid()->siteID(newTrapSite) != 2 &&
             m_world->grid()->siteID(newTrapSite) != 3 &&
           ! m_world->trapSiteIDs()->contains(newTrapSite) )
        {
            m_world->grid()->addToPotential(newTrapSite,m_world->parameters()->deltaEpsilon);
            m_world->trapSiteIDs()->push_back(newTrapSite);
        }
    }
    if ( abs(m_world->parameters()->gaussianStdev) > 0 )
    {
        for ( int i = 0; i < m_world->trapSiteIDs()->size(); i++ )
        {
            int s = m_world->trapSiteIDs()->at(i);
            double v = m_world->randomNumberGenerator()->normal(m_world->parameters()->gaussianAverg,m_world->parameters()->gaussianStdev);
            m_world->grid()->addToPotential(s,v);
        }
    }
}

void Potential::setPotentialFromFile( QString filename )
{
    QFile in(filename);
    if ( !(in.open(QIODevice::ReadOnly | QIODevice::Text)) )
    {
        qFatal("can not open file %s",qPrintable(filename));
    }
    QString contents = in.readAll();
    QStringList tokens = contents.split(QRegExp("[\\s\n]"),QString::SkipEmptyParts);

    if ( tokens.size() % 2 != 0 ) { qFatal("setPotentialFromFile does not give an even number of values (site,energy)"); }

    for ( int i = 0; i < tokens.size(); i+= 2 )
    {
        bool ok1 = true;
        bool ok2 = true;
        int site = tokens[i].toInt(&ok1);
        double energy = tokens[i+1].toFloat(&ok2);
        if ( !ok1 || !ok2 )
        {
            qDebug("setPotentialFromFile can not convert set %d (site,energy) to number: (%s,%s)",i,qPrintable(tokens[i]),qPrintable(tokens[i+1]));
            if ( !ok1 ) { qDebug("site is not int"); }
            if ( !ok2 ) { qDebug("energy is not float"); }
            qFatal("simulation terminated");
        }
        if ( site < 0 || site > m_world->grid()->volume() )
        {
            qFatal("setPotentialFromFile can not set site energy for set %d (site,energy): (%d,%f); site is out of range",i,site,energy);
        }
        m_world->grid()->addToPotential(site,energy);
    }
}

void Potential::setPotentialFromScript( QString filename )
{
    qFatal("setPotentialFromScript not implemented!");
}

void Potential::updateInteractionEnergies()
{
    //These values are used in Coulomb interaction calculations.
    boost::multi_array<double,3>& energies = m_world->interactionEnergies();
    energies.resize(boost::extents
                     [m_world->parameters()->electrostaticCutoff]
                     [m_world->parameters()->electrostaticCutoff]
                     [m_world->parameters()->electrostaticCutoff]);

    // Now calculate the numbers we need
    for (int col = 0; col < m_world->parameters()->electrostaticCutoff; ++col)
    {
        for (int row = 0; row < m_world->parameters()->electrostaticCutoff; ++row)
        {
            for (int lay = 0; lay < m_world->parameters()->electrostaticCutoff; ++lay)
            {
                double r = sqrt (col * col + row * row + lay * lay);
                if ( r > 0 && r < m_world->parameters()->electrostaticCutoff )
                {
                    energies[col][row][lay] = m_world->parameters()->elementaryCharge / r;
                }
                else
                {
                    energies[col][row][lay] = 0.0;
                }
            }
        }
    }
}

double Potential::coulombPotentialCarriers( int site )
{
    double potential = 0.0;
    for(int i = 0; i < m_world->charges()->size(); ++i)
      {
        // Potential at proposed site from other charges
        int dx = m_world->grid()->xDistancei(site, m_world->charges()->at(i)->site());
        int dy = m_world->grid()->yDistancei(site, m_world->charges()->at(i)->site());
        int dz = m_world->grid()->zDistancei(site, m_world->charges()->at(i)->site());
        if( dx < m_world->parameters()->electrostaticCutoff &&
            dy < m_world->parameters()->electrostaticCutoff &&
            dz < m_world->parameters()->electrostaticCutoff)
          {
            potential += m_world->interactionEnergies()[dx][dy][dz] * m_world->charges()->at(i)->charge();
          }
      }
    return( m_world->parameters()->electrostaticPrefactor * potential );
}

double Potential::coulombImageXPotentialCarriers( int site )
{
    double potential = 0.0;
    for(int i = 0; i < m_world->charges()->size(); ++i)
      {
        // Potential at proposed site from other charges
        int dx = m_world->grid()->xImageDistancei(site, m_world->charges()->at(i)->site());
        int dy = m_world->grid()->yDistancei(site, m_world->charges()->at(i)->site());
        int dz = m_world->grid()->zDistancei(site, m_world->charges()->at(i)->site());
        if( dx < m_world->parameters()->electrostaticCutoff &&
            dy < m_world->parameters()->electrostaticCutoff &&
            dz < m_world->parameters()->electrostaticCutoff)
          {
            potential += m_world->interactionEnergies()[dx][dy][dz] * m_world->charges()->at(i)->charge();
          }
      }
    return( m_world->parameters()->electrostaticPrefactor * potential );
}

double Potential::coulombPotentialDefects( int site )
{
    double potential = 0.0;
    for(int i = 0; i < m_world->defectSiteIDs()->size(); ++i)
      {
        // Potential at proposed site from other charges
        int dx = m_world->grid()->xDistancei(site, m_world->defectSiteIDs()->at(i));
        int dy = m_world->grid()->yDistancei(site, m_world->defectSiteIDs()->at(i));
        int dz = m_world->grid()->zDistancei(site, m_world->defectSiteIDs()->at(i));
        if( dx < m_world->parameters()->electrostaticCutoff &&
            dy < m_world->parameters()->electrostaticCutoff &&
            dz < m_world->parameters()->electrostaticCutoff)
          {
            potential += m_world->interactionEnergies()[dx][dy][dz];
          }
      }
    return( m_world->parameters()->electrostaticPrefactor * m_world->parameters ()->zDefect * potential );
}

double Potential::coulombImageXPotentialDefects( int site )
{
    double potential = 0.0;
    for(int i = 0; i < m_world->defectSiteIDs()->size(); ++i)
      {
        // Potential at proposed site from other charges
        int dx = m_world->grid()->xImageDistancei(site, m_world->defectSiteIDs()->at(i));
        int dy = m_world->grid()->yDistancei(site, m_world->defectSiteIDs()->at(i));
        int dz = m_world->grid()->zDistancei(site, m_world->defectSiteIDs()->at(i));
        if( dx < m_world->parameters()->electrostaticCutoff &&
            dy < m_world->parameters()->electrostaticCutoff &&
            dz < m_world->parameters()->electrostaticCutoff)
          {
            potential += m_world->interactionEnergies()[dx][dy][dz];
          }
      }
    return( m_world->parameters()->electrostaticPrefactor * m_world->parameters ()->zDefect * potential );
}
