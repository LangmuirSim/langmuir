#include "simulation.h"
#include "openclhelper.h"
#include "parameters.h"
#include "chargeagent.h"
#include "sourceagent.h"
#include "drainagent.h"
#include "potential.h"
#include "cubicgrid.h"
#include "logger.h"
#include "world.h"
#include "rand.h"
namespace Langmuir
{

Simulation::Simulation(World &world, QObject *parent):  QObject(parent), m_world(world)
{
    // Setup the Sites on the Grid
    initializeSites();

    // Setup Grid Potential
    // Zero potential
    m_world.potential().setPotentialZero();

    // Add Linear Potential
    m_world.potential().setPotentialLinear();

    // Add Trap Potential(assumes source and drain were created so that hetero traps don't start growing on the source / drain)
    m_world.potential().setPotentialTraps();

    // precalculate and store coulomb interaction energies
    m_world.potential().updateInteractionEnergies();

    // Generate grid image
    if(m_world.parameters().outputImage)
    {
        m_world.logger().saveTrapImage();
        m_world.logger().saveHoleImage();
        m_world.logger().saveElectronImage();
        m_world.logger().saveDefectImage();
        m_world.logger().saveImage();
    }

    // Output Field Energy
    if(m_world.parameters().outputPotential)
    {
        m_world.logger().saveElectronGridPotential();
        m_world.logger().saveHoleGridPotential();
    }

    // Output Defect IDs
    if(m_world.parameters().outputIdsAtStart)
    {
        m_world.logger().saveDefectIDs();
        m_world.logger().saveTrapIDs();
        m_world.logger().saveElectronIDs();
        m_world.logger().saveHoleIDs();
    }

    // Initialize OpenCL
    m_world.opencl().initializeOpenCL();
    if(m_world.parameters().useOpenCL)
    {
        m_world.opencl().toggleOpenCL(true);
    }
    else
    {
        m_world.opencl().toggleOpenCL(false);
    }
}

Simulation::~Simulation()
{
}

void Simulation::performIterations(int nIterations)
{
    for(int i = 0; i < nIterations; ++i)
    {
        // Attempt to transport the charges through the film
        QList<ChargeAgent*> &electrons = m_world.electrons();
        QList<ChargeAgent*> &holes = m_world.holes();

        // If using OpenCL, launch the Kernel to calculate Coulomb Interactions
        if(m_world.parameters().useOpenCL &&(electrons.size()> 0 || holes.size()> 0))
        {
            // first have the charge carriers propose future sites
            //for(int j = 0; j < charges.size(); j++)charges[j]->chooseFuture(j);
            QFuture < void > future1 = QtConcurrent::map(electrons, Simulation::chargeAgentChooseFuture);
            QFuture < void > future2 = QtConcurrent::map(holes, Simulation::chargeAgentChooseFuture);
            future1.waitForFinished();
            future2.waitForFinished();

            // tell the GPU to perform all coulomb calculations
            m_world.opencl().launchCoulombKernel2();

            //check the answer during debugging
            //m_world.opencl().compareHostAndDeviceForAllCarriers();
            //qFatal("done");

            // now use the results of the coulomb calculations to decide if the carreirs should have moved
            future1 = QtConcurrent::map(electrons, Simulation::chargeAgentDecideFuture);
            future2 = QtConcurrent::map(holes, Simulation::chargeAgentDecideFuture);
            future1.waitForFinished();
            future2.waitForFinished();
        }
        else // Not using OpenCL
        {
            // Use QtConcurrnet to parallelise the charge calculations
            QFuture < void > future1 = QtConcurrent::map(electrons, Simulation::chargeAgentIterate);
            QFuture < void > future2 = QtConcurrent::map(holes, Simulation::chargeAgentIterate);

            // We want to wait for it to finish before continuing on
            future1.waitForFinished();
            future2.waitForFinished();
        }

        // Now we are done with the charge movement, move them to the next tick!
        nextTick();

        // Perform charge injection at the source
        performInjections();

        m_world.parameters().currentStep += 1;
    }

    m_world.logger().reportFluxStream();

    // Output Coulomb Energy
    if(m_world.parameters().outputCoulomb)
    {
        m_world.opencl().launchCoulombKernel1();
        m_world.logger().saveCoulombEnergy();
    }
    // Output Carrier Positions and IDs
    if(m_world.parameters().outputIdsOnIteration)
    {
        m_world.logger().saveElectronIDs();
        m_world.logger().saveHoleIDs();
    }
}

void Simulation::placeDefects(const QList<int>& siteIDs)
{
    int count = m_world.numDefects();
    int toBePlaced = m_world.maxDefects()-count;
    int toBePlacedUsingIDs = siteIDs.size();
    int toBePlacedRandomly = toBePlaced - siteIDs.size();

    if (toBePlacedRandomly < 0)
    {
        qFatal("can not place defects;\n\tmost likely the list "
               "of defect site IDs exceeds the maximum "
               "given by defect.precentage");
    }

    // Place the defects in the siteID list
    for (int i = 0; i < toBePlacedUsingIDs; i++)
    {
        int site = siteIDs.at(i);
        if ( m_world.defectSiteIDs().contains(site) )
        {
            qFatal("can not add defect;\n\tdefect already exists");
        }
        m_world.electronGrid().registerDefect(site);
        m_world.holeGrid().registerDefect(site);
        m_world.defectSiteIDs().push_back(site);
        count++;
    }

    // Place the rest of the defects randomly
    int tries = 0;
    int maxTries = 10*(m_world.electronGrid().volume());
    for(int i = count; i < m_world.maxDefects();)
    {
        tries++;
        int site = m_world.randomNumberGenerator()
                .integer(0,m_world.electronGrid().volume()-1);

        if (!m_world.defectSiteIDs().contains(site))
        {
            m_world.electronGrid().registerDefect(site);
            m_world.holeGrid().registerDefect(site);
            m_world.defectSiteIDs().push_back(site);
            count++;
            i++;
        }

        if (tries > maxTries)
        {
            qFatal("can not seed defects;\n\texceeded max tries(%d)",
                   maxTries);
        }
    }
}

void Simulation::placeElectrons(const QList<int>& siteIDs)
{
    // Create a Source Agent for placing electrons
    ElectronSourceAgent source(m_world,Grid::NoFace);
    source.setRate(1.0);

    // Place the electrons in the site ID list
    for (int i = 0; i < siteIDs.size(); i++)
    {
        if (!source.tryToSeed(siteIDs[i]))
        {
            qFatal("can not inject electron at site %d",
                   siteIDs[i]);
        }
    }

    // Place the rest of the electrons randomly if charge seeding is on
    if (m_world.parameters().seedCharges)
    {
        int tries = 0;
        int maxTries = 10*(m_world.electronGrid().volume());
        for(int i = m_world.numElectronAgents();
            i < m_world.maxElectronAgents();)
        {
            if(source.tryToSeed()) { ++i; }
            tries++;
            if (tries > maxTries)
            {
                qFatal("can not seed electrons;\n\texceeded max tries(%d)",
                       maxTries);
            }
        }
    }
}

void Simulation::placeHoles(const QList<int>& siteIDs)
{
    // Create a Source Agent for placing holes
    HoleSourceAgent source(m_world,Grid::NoFace);
    source.setRate(1.0);

    // Place the holes in the site ID list
    for (int i = 0; i < siteIDs.size(); i++)
    {
        if (!source.tryToSeed(siteIDs[i]))
        {
            qFatal("can not inject hole at site %d",
                   siteIDs[i]);
        }
    }

    // Place the rest of the holes randomly if charge seeding is on
    if (m_world.parameters().seedCharges)
    {
        int tries = 0;
        int maxTries = 10*(m_world.holeGrid().volume());
        for(int i = m_world.numHoleAgents();
            i < m_world.maxHoleAgents();)
        {
            if(source.tryToSeed()) { ++i; }
            tries++;
            if (tries > maxTries)
            {
                qFatal("can not seed holes;\n\texceeded max tries(%d)",
                       maxTries);
            }
        }
    }
}

void Simulation::initializeSites()
{
    QList<int> defects;
    QList<int> electrons;
    QList<int> holes;
    QList<int> traps;

    if (!m_world.parameters().configurationFile.isEmpty())
    {
        QFile file;
        file.setFileName(m_world.parameters().configurationFile);
        if (!file.open(QIODevice::ReadOnly|QIODevice::Text))
        {
            qFatal("can not open data file (%s)",
                   qPrintable(m_world.parameters().configurationFile));
        }
        int count = 0;
        while (!file.atEnd())
        {
            QString line = file.readLine().trimmed().toLower();
            QStringList tokens = line.split(QRegExp("\\s"),QString::SkipEmptyParts);
            if (!(tokens.size()%2 == 0))
            {
                qFatal("invalid number of entries on line(%d) in "
                       "configuration.file\n\tline: %s",
                       count,qPrintable(line));
            }
            for (int i= 0; i < tokens.size(); i+=2)
            {
                bool ok = false;
                int site = tokens[i].toInt(&ok);
                if (!ok)
                {
                    qFatal("cannot convert site to int on line(%d) in "
                           "configuration.file\n\tline: %s",
                           count,qPrintable(line));
                }

                QList<int> *pList = 0;
                switch (tokens[i+1][0].toAscii())
                {
                case 'e': { pList = &electrons; break; }
                case 'h': { pList = &holes;     break; }
                case 'd': { pList = &defects;   break; }
                case 't': { pList = &traps;     break; }
                default:
                {
                    qFatal("invalid type on line(%d) in "
                           "configuration.file; type is "
                           "e, h, d, or t\n\tline: %s",
                           count,qPrintable(line));
                    break;
                }
                }
                if (!pList->contains(site))
                {
                    pList->push_back(site);
                }
                else
                {
                    qFatal("invalid site on line(%d) in "
                           "configuration.file; site defined "
                           "previously\n\tline: %s",
                           count,qPrintable(line));
                }
            }
            count++;
        }
    }

    // Place Defects
    placeDefects(defects);

    // place electrons on the grid
    placeElectrons(electrons);

    // place holes on the grid
    placeHoles(holes);
}

void Simulation::performInjections()
{
    for(int i = 0; i < m_world.sources().size(); i++)
    {
        m_world.sources().at(i)->tryToInject();
    }
}

void Simulation::nextTick()
{
    // Iterate over all sites to change their state
    QList<ChargeAgent*> &electrons = m_world.electrons();
    QList<ChargeAgent*> &holes = m_world.holes();

    if ( m_world.parameters().outputIdsOnDelete )
    {
        for(int i = 0; i < electrons.size(); ++i)
        {
            electrons[i]->completeTick();
            // Check if the charge was removed - then we should delete it
            if(electrons[i]->removed())
            {
                m_world.logger().report(*electrons[i]);
                delete electrons[i];
                electrons.removeAt(i);
                --i;
            }
        }
        for(int i = 0; i < holes.size(); ++i)
        {
            holes[i]->completeTick();
            // Check if the charge was removed - then we should delete it
            if(holes[i]->removed())
            {
                m_world.logger().report(*holes[i]);
                delete holes[i];
                holes.removeAt(i);
                --i;
            }
        }
        //m_world.logger().flush();
    }
    else
    {
        for(int i = 0; i < electrons.size(); ++i)
        {
            electrons[i]->completeTick();
            // Check if the charge was removed - then we should delete it
            if(electrons[i]->removed())
            {
                delete electrons[i];
                electrons.removeAt(i);
                --i;
            }
        }
        for(int i = 0; i < holes.size(); ++i)
        {
            holes[i]->completeTick();
            // Check if the charge was removed - then we should delete it
            if(holes[i]->removed())
            {
                delete holes[i];
                holes.removeAt(i);
                --i;
            }
        }
    }
}

inline void Simulation::chargeAgentIterate(ChargeAgent * chargeAgent)
{
    // This function performs a single iteration for a charge agent, only thread
    // safe calls can be made in this function. Other threads may access the
    // current state of the chargeAgent, but will not attempt to modify it.
    chargeAgent->chooseFuture();
    chargeAgent->decideFuture();
}

inline void Simulation::chargeAgentChooseFuture(ChargeAgent * chargeAgent)
{
    // same rules apply here as for chargeAgentIterate;
    chargeAgent->chooseFuture();
}

inline void Simulation::chargeAgentDecideFuture(ChargeAgent * chargeAgent)
{
    // same rules apply here as for chargeAgentIterate;
    chargeAgent->decideFuture();
}

void Simulation::equilibrated()
{
    m_world.parameters().currentStep = 0;
    foreach(FluxAgent *flux, m_world.fluxes())
    {
        flux->resetCounters();
    }
}

}
