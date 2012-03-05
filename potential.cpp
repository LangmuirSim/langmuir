#include "potential.h"
#include "parameters.h"
#include "chargeagent.h"
#include "cubicgrid.h"
#include "world.h"
#include "rand.h"

namespace Langmuir
{

Potential::Potential(World &world, QObject *parent): QObject(parent), m_world(world)
{
}

void Potential::setPotentialZero()
{
    for(int i = 0; i < m_world.electronGrid().volume(); i++)
    {
        m_world.electronGrid().setPotential(i, 0);
        m_world.holeGrid().setPotential(i, 0);
    }
}

void Potential::setPotentialLinear()
{
    double  m =(m_world.parameters().voltageRight - m_world.parameters().voltageLeft)/ double(m_world.electronGrid().xSize());
    double  b = m_world.parameters().voltageLeft;
    for(int i = 0; i < m_world.electronGrid().xSize(); i++)
    {
        for(int j = 0; j < m_world.electronGrid().ySize(); j++)
        {
            for(int k = 0; k < m_world.electronGrid().zSize(); k++)
            {
                int s = m_world.electronGrid().getIndexS(i, j, k);
                double v = m *(i + 0.5)+ b;
                m_world.electronGrid().addToPotential(s, v);
                m_world.holeGrid().addToPotential(s, v);
            }
        }
    }
}

void Potential::setPotentialTraps()
{
    // Do nothing if we shouldn't have traps
    if(m_world.parameters().trapPercentage <= 0)return;
    if(m_world.parameters().seedPercentage <  0)return;

    // Seed traps, if seed.percentage=100% then this is just placing traps normally
    int progress = int(m_world.electronGrid().volume()* m_world.parameters().trapPercentage * m_world.parameters().seedPercentage);
    while(progress >= m_world.trapSiteIDs().size())
    {
        int s = m_world.randomNumberGenerator().integer(0, m_world.electronGrid().volume()-1);

        if(! m_world.trapSiteIDs().contains(s))
        {
            m_world.electronGrid().addToPotential(s, m_world.parameters().trapPotential);
            m_world.holeGrid().addToPotential(s, -1*m_world.parameters().trapPotential);
            m_world.trapSiteIDs().push_back(s);
        }
    }

    // Do not grow traps if we aren't growing hetergeneous traps.
    if(m_world.parameters().trapPercentage <= 0)return;
    if(m_world.parameters().seedPercentage <= 0 ||
         m_world.parameters().seedPercentage == 1)return;
    progress = int(m_world.electronGrid().volume()* m_world.parameters().trapPercentage);
    while(progress >= m_world.trapSiteIDs().size())
    {
        int trapSeedIndex               = m_world.randomNumberGenerator().integer(0, m_world.trapSiteIDs().size()-1);
        int trapSeedSite                = m_world.trapSiteIDs().at(trapSeedIndex);
        QVector<int> trapSeedNeighbors  = m_world.electronGrid().neighborsSite(trapSeedSite);

        int newTrapIndex                = m_world.randomNumberGenerator().integer(0, trapSeedNeighbors.size()-1);
        int newTrapSite                 = trapSeedNeighbors[newTrapIndex];
        if(  m_world.electronGrid().agentType(newTrapSite)!= Agent::Source  &&
             m_world.electronGrid().agentType(newTrapSite)!= Agent::Source  &&
             m_world.electronGrid().agentType(newTrapSite)!= Agent::Drain   &&
             m_world.electronGrid().agentType(newTrapSite)!= Agent::Drain   &&
           ! m_world.trapSiteIDs().contains(newTrapSite))
        {
            m_world.electronGrid().addToPotential(newTrapSite, m_world.parameters().trapPotential);
            m_world.holeGrid().addToPotential(newTrapSite, -1*m_world.parameters().trapPotential);
            m_world.trapSiteIDs().push_back(newTrapSite);
        }
    }
    if(abs(m_world.parameters().gaussianStdev)> 0)
    {
        for(int i = 0; i < m_world.trapSiteIDs().size(); i++)
        {
            int s = m_world.trapSiteIDs().at(i);
            double v = m_world.randomNumberGenerator().normal(0, m_world.parameters().gaussianStdev);
            m_world.electronGrid().addToPotential(s, v);
            m_world.holeGrid().addToPotential(s, -1*v);
        }
    }
}

void Potential::setPotentialFromFile(QString fileName)
{
}

void Potential::setPotentialFromScript(QString filename)
{
    qFatal("setPotentialFromScript not implemented!");
}

void Potential::updateInteractionEnergies()
{
    //These values are used in Coulomb interaction calculations.
    boost::multi_array<double, 3>& energies = m_world.interactionEnergies();
    energies.resize(boost::extents
                    [m_world.parameters().electrostaticCutoff]
                    [m_world.parameters().electrostaticCutoff]
                    [m_world.parameters().electrostaticCutoff]);

    // Now calculate the numbers we need
    for(int col = 0; col < m_world.parameters().electrostaticCutoff; ++col)
    {
        for(int row = 0; row < m_world.parameters().electrostaticCutoff; ++row)
        {
            for(int lay = 0; lay < m_world.parameters().electrostaticCutoff; ++lay)
            {
                double r = sqrt(col * col + row * row + lay * lay);
                if(r > 0 && r < m_world.parameters().electrostaticCutoff)
                {
                    energies[col][row][lay] = m_world.parameters().electrostaticPrefactor / r;
                }
                else
                {
                    energies[col][row][lay] = 0.0;
                }
            }
        }
    }
}

double Potential::coulombPotentialElectrons(int site)
{
    double potential = 0.0;
    for(int i = 0; i < m_world.electrons().size(); ++i)
    {
        // Potential at proposed site from other charges
        int dx = m_world.electronGrid().xDistancei(site, m_world.electrons().at(i)->getCurrentSite());
        int dy = m_world.electronGrid().yDistancei(site, m_world.electrons().at(i)->getCurrentSite());
        int dz = m_world.electronGrid().zDistancei(site, m_world.electrons().at(i)->getCurrentSite());
        if(dx < m_world.parameters().electrostaticCutoff &&
                dy < m_world.parameters().electrostaticCutoff &&
                dz < m_world.parameters().electrostaticCutoff)
        {
            potential += m_world.interactionEnergies()[dx][dy][dz] * m_world.electrons().at(i)->charge();
        }
    }
    return(potential);
}

double Potential::coulombImageXPotentialElectrons(int site)
{
    double potential = 0.0;
    for(int i = 0; i < m_world.electrons().size(); ++i)
    {
        // Potential at proposed site from other charges
        int dx = m_world.electronGrid().xImageDistancei(site, m_world.electrons().at(i)->getCurrentSite());
        int dy = m_world.electronGrid().yDistancei(site, m_world.electrons().at(i)->getCurrentSite());
        int dz = m_world.electronGrid().zDistancei(site, m_world.electrons().at(i)->getCurrentSite());
        if(dx < m_world.parameters().electrostaticCutoff &&
                dy < m_world.parameters().electrostaticCutoff &&
                dz < m_world.parameters().electrostaticCutoff)
        {
            potential += m_world.interactionEnergies()[dx][dy][dz] * m_world.electrons().at(i)->charge();
        }
    }
    return(potential);
}

double Potential::coulombPotentialHoles(int site)
{
    double potential = 0.0;
    for(int i = 0; i < m_world.holes().size(); ++i)
    {
        // Potential at proposed site from other charges
        int dx = m_world.holeGrid().xDistancei(site, m_world.holes().at(i)->getCurrentSite());
        int dy = m_world.holeGrid().yDistancei(site, m_world.holes().at(i)->getCurrentSite());
        int dz = m_world.holeGrid().zDistancei(site, m_world.holes().at(i)->getCurrentSite());
        if(dx < m_world.parameters().electrostaticCutoff &&
                dy < m_world.parameters().electrostaticCutoff &&
                dz < m_world.parameters().electrostaticCutoff)
        {
            potential += m_world.interactionEnergies()[dx][dy][dz] * m_world.holes().at(i)->charge();
        }
    }
    return(potential);
}

double Potential::coulombImageXPotentialHoles(int site)
{
    double potential = 0.0;
    for(int i = 0; i < m_world.holes().size(); ++i)
    {
        // Potential at proposed site from other charges
        int dx = m_world.holeGrid().xImageDistancei(site, m_world.holes().at(i)->getCurrentSite());
        int dy = m_world.holeGrid().yDistancei(site, m_world.holes().at(i)->getCurrentSite());
        int dz = m_world.holeGrid().zDistancei(site, m_world.holes().at(i)->getCurrentSite());
        if(dx < m_world.parameters().electrostaticCutoff &&
                dy < m_world.parameters().electrostaticCutoff &&
                dz < m_world.parameters().electrostaticCutoff)
        {
            potential += m_world.interactionEnergies()[dx][dy][dz] * m_world.holes().at(i)->charge();
        }
    }
    return(potential);
}

double Potential::coulombPotentialDefects(int site)
{
    double potential = 0.0;
    for(int i = 0; i < m_world.defectSiteIDs().size(); ++i)
    {
        // Potential at proposed site from other charges
        int dx = m_world.electronGrid().xDistancei(site, m_world.defectSiteIDs().at(i));
        int dy = m_world.electronGrid().yDistancei(site, m_world.defectSiteIDs().at(i));
        int dz = m_world.electronGrid().zDistancei(site, m_world.defectSiteIDs().at(i));
        if(     dx < m_world.parameters().electrostaticCutoff &&
                dy < m_world.parameters().electrostaticCutoff &&
                dz < m_world.parameters().electrostaticCutoff)
        {
            potential += m_world.interactionEnergies()[dx][dy][dz];
        }
    }
    return(m_world.parameters().defectsCharge * potential);
}

double Potential::coulombImageXPotentialDefects(int site)
{
    double potential = 0.0;
    for(int i = 0; i < m_world.defectSiteIDs().size(); ++i)
    {
        // Potential at proposed site from other charges
        int dx = m_world.electronGrid().xImageDistancei(site, m_world.defectSiteIDs().at(i));
        int dy = m_world.electronGrid().yDistancei(site, m_world.defectSiteIDs().at(i));
        int dz = m_world.electronGrid().zDistancei(site, m_world.defectSiteIDs().at(i));
        if(dx < m_world.parameters().electrostaticCutoff &&
                dy < m_world.parameters().electrostaticCutoff &&
                dz < m_world.parameters().electrostaticCutoff)
        {
            potential += m_world.interactionEnergies()[dx][dy][dz];
        }
    }
    return(m_world.parameters().defectsCharge * potential);
}

double Potential::potentialAtSite(int site, Grid *grid, bool useCoulomb, bool useImage)
{
    double p1 = 0;
    if(grid)
    {
        p1 += grid->potential(site);
    }
    if(useCoulomb)
    {
        p1 += coulombPotentialElectrons(site);
        p1 += coulombPotentialHoles(site);
        if(m_world.parameters().coulombDefects)
        {
            p1 += coulombPotentialDefects(site);
        }
    }
    if(useImage)
    {
        p1 += coulombImageXPotentialElectrons(site);
        p1 += coulombImageXPotentialHoles(site);
        if(m_world.parameters().coulombDefects)
        {
            p1 += coulombImageXPotentialDefects(site);
        }
    }
    return p1;
}

}
