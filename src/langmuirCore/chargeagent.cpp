#include "openclhelper.h"
#include "chargeagent.h"
#include "drainagent.h"
#include "parameters.h"
#include "simulation.h"
#include "potential.h"
#include "cubicgrid.h"
#include "world.h"
#include "rand.h"

namespace LangmuirCore
{
ChargeAgent::ChargeAgent(Agent::Type type, World &world, Grid &grid, int site,QObject *parent)
    : Agent(type, world, site, parent), m_grid(grid)
{
    m_site = m_fSite;
    m_charge = 0;
    m_removed = false;
    m_lifetime = 0;
    m_pathlength = 0;
    m_openClID = 0;
    m_de = 0;
}

ElectronAgent::ElectronAgent(World &world, int site, QObject *parent)
    : ChargeAgent(Agent::Electron, world, world.electronGrid(), site, parent)
{
    m_charge = -1;
    m_grid.registerAgent(this);
}

HoleAgent::HoleAgent(World &world, int site, QObject *parent)
    : ChargeAgent(Agent::Hole, world, world.holeGrid(), site, parent)
{
    m_charge = +1;
    m_grid.registerAgent(this);
}

ChargeAgent::~ChargeAgent()
{
}

int ChargeAgent::charge()
{
    return m_charge;
}

bool ChargeAgent::removed()
{
    return m_removed;
}

int ChargeAgent::lifetime()
{
    return m_lifetime;
}

int ChargeAgent::pathlength()
{
    return m_pathlength;
}

void ChargeAgent::setOpenCLID(int id)
{
    m_openClID = id;
}

int ChargeAgent::getOpenCLID()
{
    return m_openClID;
}

void ChargeAgent::chooseFuture()
{
    // Select a proposed transport site at random
    m_fSite = m_neighbors[m_world.randomNumberGenerator().integer(0, m_neighbors.size()-1)];
    m_de = 0;
}

Grid& ChargeAgent::getGrid()
{
    return m_grid;
}

void ChargeAgent::setRemoved(const bool &status)
{
    m_removed = status;
}

void ChargeAgent::decideFuture()
{
    // Increase lifetime in existance
    m_lifetime += 1;

    switch(m_grid.agentType(m_fSite))
    {
    case Agent::Empty:
    {
        // Potential difference between sites
        double pd = m_grid.potential(m_fSite)- m_grid.potential(m_site);
        pd *= m_charge;

        // Coulomb interactions
        // Don't worry, it's zero if coulomb interactions are off
        pd += m_de;

        // Calculate the coupling constant...
        int dx = m_grid.xDistancei(m_site, m_fSite);
        int dy = m_grid.yDistancei(m_site, m_fSite);
        int dz = m_grid.zDistancei(m_site, m_fSite);
        double coupling = m_world.couplingConstants()[dx][dy][dz];

        // Metropolis criterion
        if(m_world.randomNumberGenerator().metropolisWithCoupling(
                 pd,
                 m_world.parameters().inverseKT,
                 coupling))
        {
            // Accept move - increase distance traveled
            m_pathlength += 1;
            return;
        }
        else
        {
            // Reject move
            m_fSite = m_site;
        }
        return;
        break;
    }

    case Agent::Drain:
    {
        DrainAgent *drain = dynamic_cast<DrainAgent*>(m_grid.agentAddress(m_fSite));
        if(drain)
        {
            if(drain->tryToAccept(this))
            {
                m_pathlength += 1;
                break;
            }
        }
        else
        {
            qFatal("langmuir: can not cast pointer to DrainAgent");
        }
        // Reject the move
        m_fSite = m_site;
        break;
    }

    default:
    {
        // Invalid site proposed(Defect, Electron, Hole, Source)
        m_fSite = m_site;
        break;
    }

    }
    return;
}

void ChargeAgent::completeTick()
{
    // If the charge was removed by some other means (recombination)...
    if (m_removed)
    {
        m_grid.unregisterAgent(this);
        return;
    }

    // If the charge moved...
    if(m_site != m_fSite)
    {   
        // If the future site is empty move along
        if(m_grid.agentType(m_fSite)== Agent::Empty)
        {            
            // Leave old site
            m_grid.unregisterAgent(this);

            // Enter new site
            m_site = m_fSite;
            m_grid.registerAgent(this);
            return;
        }

        // If the future site is the drain then remove charge
        if(m_grid.agentType(m_fSite)== Agent::Drain)
        {
            m_grid.unregisterAgent(this);
            m_removed = true;
            return;
        }

        // Abort the move - the site was not empty
        m_fSite = m_site;
        return;
    }
}

double ChargeAgent::coulombInteraction()
{
    if(m_world.parameters().useOpenCL)
    {
        coulombGPU();
        return m_de;
    }
    else
    {
        coulombCPU();
        return m_de;
    }
}

void ChargeAgent::coulombCPU()
{
    double p1 = 0;
    double p2 = 0;

    // Compute self interaction
    //int dx = m_grid.xDistancei(m_site, m_fSite);
    //int dy = m_grid.yDistancei(m_site, m_fSite);
    //int dz = m_grid.zDistancei(m_site, m_fSite);
    double self = m_world.sI()[1][0][0] * m_charge;

    // Gaussian charges
    if (m_world.parameters().coulombGaussianSigma > 0)
    {
        // Electrons
        p1 += m_world.potential().gaussE(m_site);
        p2 += m_world.potential().gaussE(m_fSite);

        // Holes
        p1 += m_world.potential().gaussH(m_site);
        p2 += m_world.potential().gaussH(m_fSite);

        // Charged defects
        if(m_world.parameters().defectsCharge != 0)
        {
            p1 += m_world.potential().gaussD(m_site);
            p2 += m_world.potential().gaussD(m_fSite);
        }
    }
    // Normal charges
    else
    {
        // Electrons
        p1 += m_world.potential().coulombE(m_site);
        p2 += m_world.potential().coulombE(m_fSite);

        // Holes
        p1 += m_world.potential().coulombH(m_site);
        p2 += m_world.potential().coulombH(m_fSite);

        // Charged defects
        if(m_world.parameters().defectsCharge != 0)
        {
            p1 += m_world.potential().coulombD(m_site);
            p2 += m_world.potential().coulombD(m_fSite);
        }
    }

    // Remove self interaction
    p2 -= self;

    //When holes and electrons on on the same site the interaction is not zero
    p2 += bindingPotential(m_fSite);
    p1 += bindingPotential(m_site);

    m_de = m_charge * (p2 - p1);
}

void ChargeAgent::coulombGPU()
{
    double p1 = 0;
    double p2 = 0;

    // Assuming the GPU calculation output was copied to the CPU already
    p1 += m_world.opencl().getOutputHost(m_openClID);
    p2 += m_world.opencl().getOutputHostFuture(m_openClID);

    // Compute self interaction
    //int dx = m_grid.xDistancei(m_site, m_fSite);
    //int dy = m_grid.yDistancei(m_site, m_fSite);
    //int dz = m_grid.zDistancei(m_site, m_fSite);
    double self = m_world.sI()[1][0][0] * m_charge;

    // Remove self interaction
    p2 -= self;

    //When holes and electrons on on the same site the interaction is not zero
    p2 += bindingPotential(m_fSite);
    p1 += bindingPotential(m_site);

    m_de = m_charge *(p2 - p1);
}

void ChargeAgent::compareCoulomb()
{
    double SELF = m_world.iR()[1][0][0] * m_charge *
                  m_world.parameters().electrostaticPrefactor;

    // GPU
    double GPU1 = m_world.opencl().getOutputHost(m_openClID);
    double GPU2 = m_world.opencl().getOutputHostFuture(m_openClID) - SELF;
    double GPU  = m_charge * (GPU2 - GPU1);

    // Electrons
    double CPU1_E = 0.0;
    double CPU2_E = 0.0;

    // Holes
    double CPU1_H = 0.0;
    double CPU2_H = 0.0;

    // Charged defects
    double CPU1_D = 0.0;
    double CPU2_D = 0.0;

    if (m_world.parameters().coulombGaussianSigma > 0)
    {
        // Electrons
        CPU1_E = m_world.potential().gaussE(m_site);
        CPU2_E = m_world.potential().gaussE(m_fSite);

        // Holes
        CPU1_H = m_world.potential().gaussH(m_site);
        CPU2_H = m_world.potential().gaussH(m_fSite);

        // Charged defects
        if(m_world.parameters().defectsCharge != 0)
        {
            CPU1_D = m_world.potential().gaussD(m_site);
            CPU2_D = m_world.potential().gaussD(m_fSite);
        }
    }
    else
    {
        // Electrons
        CPU1_E = m_world.potential().coulombE(m_site);
        CPU2_E = m_world.potential().coulombE(m_fSite);

        // Holes
        CPU1_H = m_world.potential().coulombH(m_site);
        CPU2_H = m_world.potential().coulombH(m_fSite);

        // Charged defects
        if(m_world.parameters().defectsCharge != 0)
        {
            CPU1_D = m_world.potential().coulombD(m_site);
            CPU2_D = m_world.potential().coulombD(m_fSite);
        }
    }

    // CPU
    double CPU1 = CPU1_E + CPU1_H + CPU1_D;
    double CPU2 = CPU2_E + CPU2_H + CPU2_D - SELF;
    double CPU  = m_charge * (CPU2 - CPU1);

    // CHANGE
    double DIFF   = GPU  - CPU;
    double DIFF1  = GPU1 - CPU1;
    double DIFF2  = GPU2 - CPU2;
    double PDIFF  = fabs(DIFF )/(0.5 *(fabs(CPU )+ fabs(GPU ))) * 100.0;
    double PDIFF1 = fabs(DIFF1)/(0.5 *(fabs(CPU1)+ fabs(GPU1))) * 100.0;
    double PDIFF2 = fabs(DIFF2)/(0.5 *(fabs(CPU2)+ fabs(GPU2))) * 100.0;

    bool FAILED  = DIFF   > 1e-4;
    bool FAILED1 = PDIFF1 > 1e-4;
    bool FAILED2 = PDIFF2 > 1e-4;

    // RATIO
    double RATIO  = GPU  / CPU ;
    double RATIO1 = GPU1 / CPU1;
    double RATIO2 = GPU2 / CPU2;

    int    w =  23;
    int    p =  15;
    char fmt = 'e';

    qDebug() << qPrintable(QString("TYPE_1 : %1")
                           .arg(toQString(m_grid.agentType(m_site))));
    qDebug() << qPrintable(QString("SITE_1 : %2, %3, %4, %5")
                           .arg(m_site, 4)
                           .arg(m_grid.getIndexX(m_site), 4)
                           .arg(m_grid.getIndexY(m_site), 4)
                           .arg(m_grid.getIndexZ(m_site), 4));
    qDebug() << qPrintable(QString("TYPE_2 : %1")
                           .arg(toQString(m_grid.agentType(m_fSite))));
    qDebug() << qPrintable(QString("SITE_2 : %2, %3, %4, %5")
                           .arg(m_fSite, 4)
                           .arg(m_grid.getIndexX(m_fSite), 4)
                           .arg(m_grid.getIndexY(m_fSite), 4)
                           .arg(m_grid.getIndexZ(m_fSite), 4));
    qDebug() << qPrintable(QString("CPU    : %1").arg(CPU, 23, 'e', 15));
    qDebug() << qPrintable(QString("V_1    : %1 (E=%2, H=%3, D=%4)")
                           .arg(CPU1  , w, fmt, p)
                           .arg(CPU1_E, w, fmt, p)
                           .arg(CPU1_H, w, fmt, p)
                           .arg(CPU1_D, w, fmt, p));
    qDebug() << qPrintable(QString("V_2    : %1 (E=%2, H=%3, D=%4)")
                           .arg(CPU2  , w, fmt, p)
                           .arg(CPU2_E, w, fmt, p)
                           .arg(CPU2_H, w, fmt, p)
                           .arg(CPU2_D, w, fmt, p));
    qDebug() << qPrintable(QString("GPU    : %1")
                           .arg(GPU   , w, fmt, p));
    qDebug() << qPrintable(QString("V_1    : %1")
                           .arg(GPU1  , w, fmt, p));
    qDebug() << qPrintable(QString("V_2    : %1")
                           .arg(GPU2  , w, fmt, p));
    qDebug() << qPrintable(QString("DIFF   : %1 (%2 %)")
                           .arg( DIFF , w, fmt, p)
                           .arg(PDIFF , w, fmt, p));
    qDebug() << qPrintable(QString("DIFF_1 : %1 (%2 %)")
                           .arg( DIFF1, w, fmt, p)
                           .arg(PDIFF1, w, fmt, p));
    qDebug() << qPrintable(QString("DIFF_2 : %1 (%2 %)")
                           .arg( DIFF2, w, fmt, p)
                           .arg(PDIFF2, w, fmt, p));
    if (FAILED)
    {
        qDebug() << qPrintable(QString("RATIO  : %1 FAILED!!!")
                           .arg(RATIO , w, fmt, p));
    }
    else
    {
        qDebug() << qPrintable(QString("RATIO  : %1 PASSED")
                           .arg(RATIO , w, fmt, p));
    }

    if (FAILED1)
    {
        qDebug() << qPrintable(QString("RATIO1 : %1 FAILED!!!")
                           .arg(RATIO1, w, fmt, p));
    }
    else
    {
        qDebug() << qPrintable(QString("RATIO1 : %1 PASSED")
                           .arg(RATIO1, w, fmt, p));
    }

    if (FAILED2)
    {
        qDebug() << qPrintable(QString("RATIO2 : %1 FAILED!!!")
                           .arg(RATIO2, w, fmt, p));
    }
    else
    {
        qDebug() << qPrintable(QString("RATIO2 : %1 PASSED")
                           .arg(RATIO2, w, fmt, p));
    }

    if(FAILED || FAILED1 || FAILED2)
    {
        qFatal("langmuir: CPU and GPU disagree for %s %d",
                 qPrintable(toQString(m_type)),
                 m_openClID);
    }
    qDebug() << "--------------------------------------------------------------------------------";
}

double HoleAgent::bindingPotential(int site)
{
    if(m_world.electronGrid().agentType(site)== Agent::Electron)
    {
        return m_world.sI()[1][0][0] - m_world.parameters().excitonBinding;
        //return -0.5;
    }
    return 0.0;
}

double ElectronAgent::bindingPotential(int site)
{
    if(m_world.holeGrid().agentType(site)== Agent::Hole)
    {
        return m_world.sI()[1][0][0] + m_world.parameters().excitonBinding;
        //return 0.5;
    }
    return 0.0;
}

Agent::Type HoleAgent::otherType()
{
    return Agent::Electron;
}

Agent::Type ElectronAgent::otherType()
{
    return Agent::Hole;
}

Grid& HoleAgent::otherGrid()
{
    return m_world.electronGrid();
}

Grid& ElectronAgent::otherGrid()
{
    return m_world.holeGrid();
}

}
