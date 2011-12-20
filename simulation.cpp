#include "simulation.h"
#include "openclhelper.h"
#include "inputparser.h"
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
    Simulation::Simulation (SimulationParameters * par, int id) :  m_world(0), m_tick(0), m_id(id)
    {
        // Create the World Object
        m_world = new World(par);

        // Tell World about this simulation
        m_world->setSimulation(this);

        // Set up Drains on left and right of the grid
        this->setUpSources();

        // Set up Sources on left and right of the grid
        this->setUpDrains();

        // Place Defects
        this->createDefects();

        // Setup Grid Potential
        // Zero potential
        m_world->potential()->setPotentialZero();

        // Add Linear Potential
        if ( m_world->parameters()->potentialLinear ) { m_world->potential()->setPotentialLinear(); }

        // Add Trap Potential ( assumes source and drain were created so that hetero traps don't start growing on the source / drain )
        m_world->potential()->setPotentialTraps();

        // precalculate and store coulomb interaction energies
        m_world->potential()->updateInteractionEnergies();

        // place charges on the grid randomly ( assumes source / drain were created )
        if (m_world->parameters()->gridCharge) seedCharges();

        // Generate grid image
        if (m_world->parameters()->outputGrid)
        {
            m_world->logger()->saveTrapImageToFile(QString("trap-%1-%2-%3.png")
                   .arg(m_world->parameters()->trapPercentage*100.0)
                   .arg(m_world->parameters()->seedPercentage*100.0)
                   .arg(m_id));
        }

        // Output Field Energy
        if (m_world->parameters()->outputFieldPotential)
        {
            m_world->logger()->saveFieldEnergyToFile(QString("field-%1.dat").arg(m_id));
        }

        // Output Defect IDs
        if (m_world->parameters()->outputDefectIDs)
        {
            m_world->logger()->saveDefectIDsToFile(QString("defectIDs-%1.dat").arg(m_id));
        }

        // Output Trap IDs
        if (m_world->parameters()->outputTrapIDs)
        {
            m_world->logger()->saveTrapIDsToFile(QString("trapIDs-%1.dat").arg(m_id));
        }

        // Initialize OpenCL
        m_world->opencl()->initializeOpenCL();
        if ( m_world->parameters()->useOpenCL )
        {
            m_world->opencl()->toggleOpenCL(true);
        }
        else
        {
            m_world->opencl()->toggleOpenCL(false);
        }
    }

    Simulation::~Simulation ()
    {
        if ( m_world != 0 )
        {
            delete m_world;
        }
    }

    void Simulation::createDefects()
    {
        QVector<int> sourceNeighbors = m_world->sourceL()->getNeighbors();
        QVector<int> drainNeighnors = m_world->drainR()->getNeighbors();
        for (int i = 0; i < m_world->electronGrid()->volume (); ++i)
        {
            if (m_world->randomNumberGenerator()->random() < m_world->parameters()->defectPercentage)
            {
                m_world->electronGrid()->setAgentType(i,Agent::Defect);
                m_world->holeGrid()->setAgentType(i,Agent::Defect);
                m_world->defectSiteIDs()->push_back(i);
                int sourceNeighborsHasDefect = sourceNeighbors.indexOf(i);
                int drainNeighnorsHasDefect = drainNeighnors.indexOf(i);
                if ( sourceNeighborsHasDefect != -1 ) { sourceNeighbors.remove(sourceNeighborsHasDefect); }
                if ( drainNeighnorsHasDefect != -1 ) { drainNeighnors.remove(drainNeighnorsHasDefect); }
            }
            else
            {
                m_world->electronGrid()->setAgentType(i,Agent::Empty);
            }
        }
        m_world->sourceL()->setNeighbors(sourceNeighbors);
        m_world->drainR()->setNeighbors(drainNeighnors);
    }

    void Simulation::seedCharges()
    {
        for (int i = 0; i < m_world->sourceL()->maxHoles();)
        {
            if (m_world->sourceL()->seedHole())
            {
                ++i;
            }
        }
        for (int i = 0; i < m_world->sourceL()->maxElectrons();)
        {
            if (m_world->sourceL()->seedElectron())
            {
                ++i;
            }
        }
    }

    void Simulation::setUpDrains()
    {
        Grid &EGrid = *m_world->electronGrid();
        Grid &HGrid = *m_world->holeGrid();

        int IDLeft  = EGrid.getIndexDrainL();
        int IDRight = EGrid.getIndexDrainR();

        DrainAgent *DrainLeft  = m_world->drainL();
        DrainAgent *DrainRight = m_world->drainR();

        QVector<int> leftNeighbors = EGrid.sliceIndex(0,1,0,EGrid.height(),0,EGrid.depth());
        QVector<int> rightNeighbors = EGrid.sliceIndex(EGrid.width()-1,EGrid.width(),0,EGrid.height(),0,EGrid.depth());

        DrainLeft->setSite(IDLeft);
        EGrid.setAgentAddress(IDLeft,DrainLeft);
        EGrid.setAgentType(IDLeft,Agent::DrainL);
        HGrid.setAgentAddress(IDLeft,DrainLeft);
        HGrid.setAgentType(IDLeft,Agent::DrainL);
        DrainLeft->setNeighbors(leftNeighbors);

        DrainRight->setSite(IDRight);
        EGrid.setAgentAddress(IDRight,DrainRight);
        EGrid.setAgentType(IDRight,Agent::DrainR);
        HGrid.setAgentAddress(IDRight,DrainRight);
        HGrid.setAgentType(IDRight,Agent::DrainR);
        DrainRight->setNeighbors(rightNeighbors);
    }

    void Simulation::setUpSources()
    {
        Grid &EGrid = *m_world->electronGrid();
        Grid &HGrid = *m_world->holeGrid();

        int IDLeft  = EGrid.getIndexSourceL();
        int IDRight = EGrid.getIndexSourceR();

        SourceAgent *SourceLeft  = m_world->sourceL();
        SourceAgent *SourceRight = m_world->sourceR();

        QVector<int> leftNeighbors = EGrid.sliceIndex(0,1,0,EGrid.height(),0,EGrid.depth());
        QVector<int> rightNeighbors = EGrid.sliceIndex(EGrid.width()-1,EGrid.width(),0,EGrid.height(),0,EGrid.depth());

        SourceLeft->setSite(IDLeft);
        EGrid.setAgentAddress(IDLeft,SourceLeft);
        EGrid.setAgentType(IDLeft,Agent::SourceL);
        HGrid.setAgentAddress(IDLeft,SourceLeft);
        HGrid.setAgentType(IDLeft,Agent::SourceL);
        SourceLeft->setNeighbors(leftNeighbors);

        SourceRight->setSite(IDRight);
        EGrid.setAgentAddress(IDRight,SourceRight);
        EGrid.setAgentType(IDRight,Agent::SourceR);
        HGrid.setAgentAddress(IDRight,SourceRight);
        HGrid.setAgentType(IDRight,Agent::SourceR);
        SourceRight->setNeighbors(rightNeighbors);
    }

} // End namespace Langmuir
