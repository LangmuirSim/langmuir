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
    m_coupling[ Agent::Electron ][ Agent::Empty  ] = 0.333;
    m_coupling[ Agent::Hole     ][ Agent::Empty  ] = 0.333;
    m_coupling[ Agent::Electron ][ Agent::DrainL ] = 0.900;
    m_coupling[ Agent::Electron ][ Agent::DrainR ] = 0.900;
    m_coupling[ Agent::Hole     ][ Agent::DrainL ] = 0.900;
    m_coupling[ Agent::Hole     ][ Agent::DrainR ] = 0.900;

    m_parameters = par;
    m_rand = new Random(m_parameters->randomSeed);
    m_electronGrid = new CubicGrid(m_parameters->gridWidth, m_parameters->gridHeight, m_parameters->gridDepth);
    m_holeGrid = new CubicGrid(m_parameters->gridWidth, m_parameters->gridHeight, m_parameters->gridDepth);
    m_potential = new Potential(this);
    m_logger = new Logger(this);
    m_ocl = new OpenClHelper(this);
    m_sourceL = new SourceAgent(Agent::SourceL,this,0);
    m_sourceR = new SourceAgent(Agent::SourceR,this,0);
    m_drainL = new DrainAgent(Agent::DrainL,this,0);
    m_drainR = new DrainAgent(Agent::DrainR,this,0);
  }

  World::~World()
  {
   delete m_rand;
   delete m_potential;
   delete m_sourceL;
   delete m_sourceR;
   delete m_drainL;
   delete m_drainR;
   delete m_electronGrid;
   delete m_holeGrid;
   delete m_logger;
   delete m_ocl;
  }
}
