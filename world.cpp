#include "inputparser.h"
#include "openclhelper.h"
#include "chargeagent.h"
#include "sourceagent.h"
#include "drainagent.h"
#include "potential.h"
#include "cubicgrid.h"
#include "logger.h"
#include "world.h"
#include "rand.h"

namespace Langmuir {

World::World( SimulationParameters *par )
{
    m_coupling.resize(boost::extents[Agent::SIZE][Agent::SIZE]);

    m_coupling[ Agent::Electron        ][ Agent::Empty    ] = 0.333;
    m_coupling[ Agent::Hole            ][ Agent::Empty    ] = 0.333;

    m_parameters = par;
    m_rand = new Random(m_parameters->randomSeed);
    m_electronGrid = new Grid(m_parameters->gridWidth, m_parameters->gridHeight, m_parameters->gridDepth);
    m_holeGrid = new Grid(m_parameters->gridWidth, m_parameters->gridHeight, m_parameters->gridDepth);
    m_potential = new Potential(this);
    m_logger = new Logger(this);
    m_ocl = new OpenClHelper(this);

    ElectronSourceAgent *p1 = new ElectronSourceAgent(this,Grid::Left,0,0.9,1);
    m_sources.push_back( p1 );

    ElectronDrainAgent *p2 = new ElectronDrainAgent(this,Grid::Right,0,0.9,1);
    m_drains.push_back( p2 );
}

World::~World()
{
    for ( int i = 0; i < m_sources.size(); i++ )
    {
        delete m_sources[i];
    }
    delete m_rand;
    delete m_potential;
    delete m_electronGrid;
    delete m_holeGrid;
    delete m_logger;
    delete m_ocl;
}
}
