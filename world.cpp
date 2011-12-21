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
    m_coupling[ Agent::Electron ][ Agent::Empty    ] = 0.333;
    m_coupling[ Agent::Hole     ][ Agent::Empty    ] = 0.333;
    m_coupling[ Agent::Electron ][ Agent::DrainL   ] = 0.900;
    m_coupling[ Agent::Electron ][ Agent::DrainR   ] = 0.900;
    m_coupling[ Agent::Hole     ][ Agent::DrainL   ] = 0.900;
    m_coupling[ Agent::Hole     ][ Agent::DrainR   ] = 0.900;

    //m_coupling[ Agent::HoleSource  ][ Agent::Electron ] = 0.900;
    //m_coupling[ Agent::ElectronSource  ][ Agent::Electron ] = 0;
    //m_coupling[ Agent::ExcitonSource    ][ Agent::Electron ] = 0.1;
    //m_coupling[ Agent::HoleSource  ][ Agent::Hole     ] = 0;
    //m_coupling[ Agent::ElectronSource  ][ Agent::Hole     ] = 0;
    //m_coupling[ Agent::ExcitonSource    ][ Agent::Hole     ] = 0.1;

    m_parameters = par;
    m_rand = new Random(m_parameters->randomSeed);
    m_electronGrid = new CubicGrid(m_parameters->gridWidth, m_parameters->gridHeight, m_parameters->gridDepth);
    m_holeGrid = new CubicGrid(m_parameters->gridWidth, m_parameters->gridHeight, m_parameters->gridDepth);
    m_potential = new Potential(this);
    m_logger = new Logger(this);
    m_ocl = new OpenClHelper(this);
    m_holeSource = new HoleSourceAgent(this);
    m_electronSource = new ElectronSourceAgent(this);
    m_excitonSource = new ExcitonSourceAgent(this);
    m_drainL = new DrainAgent(Agent::DrainL,this,0);
    m_drainR = new DrainAgent(Agent::DrainR,this,0);

  }

  World::~World()
  {
   delete m_rand;
   delete m_potential;
   delete m_holeSource;
   delete m_electronSource;
   delete m_drainL;
   delete m_drainR;
   delete m_electronGrid;
   delete m_holeGrid;
   delete m_logger;
   delete m_ocl;
  }
}
