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

    /*
    m_coupling[ Agent::Electron        ][ Agent::HoleDrainL   ] = 0.900;
    m_coupling[ Agent::Electron        ][ Agent::HoleDrainR   ] = 0.900;

    m_coupling[ Agent::Hole            ][ Agent::HoleDrainL   ] = 0.900;
    m_coupling[ Agent::Hole            ][ Agent::HoleDrainR   ] = 0.900;
    */

    m_coupling[ Agent::HoleSourceL     ][ Agent::Empty    ] = 0.000;
    m_coupling[ Agent::HoleSourceR     ][ Agent::Empty    ] = 0.000;

    m_coupling[ Agent::ElectronSourceL ][ Agent::Empty    ] = 0.000;
    m_coupling[ Agent::ElectronSourceR ][ Agent::Empty    ] = 0.000;

    m_coupling[ Agent::ExcitonSource   ][ Agent::Empty    ] = 0.050;

    m_parameters = par;
    m_rand = new Random(m_parameters->randomSeed);
    m_electronGrid = new CubicGrid(m_parameters->gridWidth, m_parameters->gridHeight, m_parameters->gridDepth);
    m_holeGrid = new CubicGrid(m_parameters->gridWidth, m_parameters->gridHeight, m_parameters->gridDepth);
    m_potential = new Potential(this);
    m_logger = new Logger(this);
    m_ocl = new OpenClHelper(this);
    m_holeSourceL = new HoleSourceAgentLeft(this);
    m_holeSourceR = new HoleSourceAgentRight(this);
    m_electronSourceL = new ElectronSourceAgentLeft(this);
    m_electronSourceR = new ElectronSourceAgentRight(this);
    m_excitonSource = new ExcitonSourceAgent(this);
    m_holeDrainL = new HoleDrainAgentLeft(this);
    m_holeDrainR = new HoleDrainAgentRight(this);
    m_electronDrainL = new ElectronDrainAgentLeft(this);
    m_electronDrainR = new ElectronDrainAgentRight(this);
  }

  World::~World()
  {
   delete m_rand;
   delete m_potential;
   delete m_holeSourceL;
   delete m_holeSourceR;
   delete m_electronSourceL;
   delete m_electronSourceR;
   delete m_excitonSource;
   delete m_holeDrainL;
   delete m_holeDrainR;
   delete m_electronDrainL;
   delete m_electronDrainR;
   delete m_electronGrid;
   delete m_holeGrid;
   delete m_logger;
   delete m_ocl;
  }
}
