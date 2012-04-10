#include "potential.h"
#include "parameters.h"
#include "chargeagent.h"
#include "cubicgrid.h"
#include "world.h"
#include "rand.h"
#include <cmath>

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

void Potential::setPotentialGate()
{
    if ( m_world.parameters().gridZ == 1 || m_world.parameters().slopeZ == 0 )
    {
        return;
    }
    for(int i = 0; i < m_world.electronGrid().xSize(); i++)
    {
        for(int j = 0; j < m_world.electronGrid().ySize(); j++)
        {
            for(int k = 0; k < m_world.electronGrid().zSize(); k++)
            {
                int s = m_world.electronGrid().getIndexS(i, j, k);
                double v = m_world.parameters().slopeZ *(k + 0.5);
                m_world.electronGrid().addToPotential(s, v);
                m_world.holeGrid().addToPotential(s, v);
            }
        }
    }
}

void Potential::setPotentialTraps(const QList<int> &trapIDs, const QList<double> &trapPotentials)
{
    if ( m_world.numTraps() != 0 )
    {
        qFatal("traps have already been created; can not call setPotentialTraps");
    }

    int toBePlacedTotal    = m_world.maxTraps();
    int toBePlacedForced   = trapIDs.size();
    int toBePlacedRandomly = toBePlacedTotal - toBePlacedForced;
    int toBePlacedSeeds    = toBePlacedRandomly * m_world.parameters().seedPercentage;
    int toBePlacedGrown    = toBePlacedRandomly - toBePlacedSeeds;

    // This happens when all traps are forced
    if (toBePlacedRandomly <= 0)
    {
        toBePlacedSeeds = 0;
        toBePlacedGrown = 0;
    }

    if (toBePlacedSeeds == 0 && toBePlacedGrown > 0)
    {
        toBePlacedSeeds = toBePlacedGrown;
        toBePlacedGrown = 0;
    }

    QString message =
            QString("parameters:               \n"
                    "        total: %1         \n"
                    "       forced: %2         \n"
                    "       random: %3         \n"
                    "       seeded: %4         \n"
                    "        grown: %5         \n"
                    "                          \n"
                    " total  = forced + random \n"
                    " random = seeded + grown  \n")
            .arg(toBePlacedTotal,20)
            .arg(toBePlacedForced,20)
            .arg(toBePlacedRandomly,20)
            .arg(toBePlacedSeeds,20)
            .arg(toBePlacedGrown,20);

    if (toBePlacedTotal < 0 ||
            toBePlacedForced < 0 ||
            toBePlacedRandomly < 0 ||
            toBePlacedSeeds < 0 ||
            toBePlacedGrown < 0 ||
            toBePlacedForced > toBePlacedTotal ||
            toBePlacedRandomly > toBePlacedTotal ||
            toBePlacedSeeds > toBePlacedTotal ||
            toBePlacedGrown > toBePlacedTotal)
    {
        qFatal("invalid trap %s",qPrintable(message));
    }

    if ( toBePlacedForced > 0 &&
         trapPotentials.size() != 0 &&
         trapPotentials.size() != trapIDs.size() )
    {
        qFatal("invalid trap %s\n forced number of ids (%d) "
               "does not match forced number of passed "
               "potentials (%d)",
               qPrintable(message),
               trapIDs.size(),
               trapPotentials.size());
    }

    // We do not alter the grid potential until we are done
    QList<double> potentials;
    QList<int>         traps;

    // First, place the forced traps
    for (int i = 0; i < toBePlacedForced; i++)
    {
        if (trapPotentials.size() != 0)
        {
            potentials.push_back(trapPotentials.at(i));
        }
        else
        {
            potentials.push_back(m_world.parameters().trapPotential);
        }
        traps.push_back(trapIDs.at(i));
    }

    // Now place traps randomly
    QList<double> randomPotentials;
    QList<int>    randomIDs;

    // Place the seeds
    int progress = 0;
    while (progress < toBePlacedSeeds)
    {
        int s = m_world.randomNumberGenerator().integer(0, m_world.electronGrid().volume()-1);
        if ( ! randomIDs.contains(s) && ! traps.contains(s) )
        {
            randomIDs.push_back(s);
            randomPotentials.push_back(m_world.parameters().trapPotential);
            ++progress;
        }
    }

    // Place the grown (if 0, does nothing)
    progress = 0;
    while (progress < toBePlacedGrown)
    {
        int trapSeedIndex               = m_world.randomNumberGenerator().integer(0, randomIDs.size()-1);
        int trapSeedSite                = randomIDs.at(trapSeedIndex);
        QVector<int> trapSeedNeighbors  = m_world.electronGrid().neighborsSite(trapSeedSite,1);

        int newTrapIndex                = m_world.randomNumberGenerator().integer(0, trapSeedNeighbors.size()-1);
        int newTrapSite                 = trapSeedNeighbors[newTrapIndex];
        if(  m_world.electronGrid().agentType(newTrapSite)!= Agent::Source  &&
             m_world.electronGrid().agentType(newTrapSite)!= Agent::Drain  &&
             m_world.holeGrid().agentType(newTrapSite)!= Agent::Source   &&
             m_world.holeGrid().agentType(newTrapSite)!= Agent::Drain   &&
             ! randomIDs.contains(newTrapSite) &&
             ! traps.contains(newTrapSite) )
        {
            randomIDs.push_back(newTrapSite);
            randomPotentials.push_back(m_world.parameters().trapPotential);
            ++progress;
        }
    }

    // Apply a deviation to the trap energies (only the randomly generated ones)
    if(abs(m_world.parameters().gaussianStdev)> 0)
    {
        for(int i = 0; i < randomIDs.size(); i++)
        {
            double v = m_world.randomNumberGenerator().normal(0, m_world.parameters().gaussianStdev);
            randomPotentials[i] = randomPotentials[i] + v;
        }
    }

    // Save the trap ids and potentials
    for (int i = 0; i < randomIDs.size(); i++)
    {
        traps.push_back(randomIDs.at(i));
        potentials.push_back(randomPotentials.at(i));
    }
    m_world.trapSiteIDs() = traps;
    m_world.trapSitePotentials() = potentials;

    // Now update the grids
    for (int i = 0; i < traps.size(); i++)
    {
        int s = traps.at(i);
        double v = potentials.at(i);
        m_world.electronGrid().addToPotential(s,v);
        m_world.holeGrid().addToPotential(s,v);
    }
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

void Potential::updateCouplingConstants()
{
    //These values are used for moving between sites
    boost::multi_array<double, 3>& constants = m_world.couplingConstants();
    constants.resize(boost::extents
                     [m_world.parameters().hoppingRange+1]
                     [m_world.parameters().hoppingRange+1]
                     [m_world.parameters().hoppingRange+1]);

    //Assuming coupling(r=1) is 1/3 and coupling(r=2) is 1/27 for now
    double c1 = 1.0 /  3.0;
    double c2 = 1.0 / 27.0;
    for (int dx = 0; dx < m_world.parameters().hoppingRange+1; dx++)
    {
        for (int dy = 0; dy < m_world.parameters().hoppingRange+1; dy++)
        {
            for (int dz = 0; dz < m_world.parameters().hoppingRange+1; dz++)
            {
                double r = sqrt(dx*dx + dy*dy + dz*dz);
                double c = c1 * pow(3.0, r - 1) * pow(c2, r - 1);
                constants[dx][dy][dz] = c;
                //qDebug() << dx << dy << dz << constants[dx][dy][dz];
            }
        }
    }
    constants[0][0][0] = 0;
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
        if(m_world.parameters().defectsCharge != 0)
        {
            p1 += coulombPotentialDefects(site);
        }
    }
    if(useImage)
    {
        p1 += coulombImageXPotentialElectrons(site);
        p1 += coulombImageXPotentialHoles(site);
        if(m_world.parameters().defectsCharge != 0)
        {
            p1 += coulombImageXPotentialDefects(site);
        }
    }
    return p1;
}

}
