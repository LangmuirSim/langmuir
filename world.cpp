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
    m_coupling.resize(boost::extents[4][4]);
    m_coupling[0][0] = 0.333;  m_coupling[0][1] = 0.000;  m_coupling[0][2] = 0.333;  m_coupling[0][3] = 0.333;
    m_coupling[1][0] = 0.000;  m_coupling[1][1] = 0.000;  m_coupling[1][2] = 0.000;  m_coupling[1][3] = 0.000;
    m_coupling[2][0] = 0.333;  m_coupling[2][1] = 0.000;  m_coupling[2][2] = 0.002;  m_coupling[2][3] = 0.002;
    m_coupling[3][0] = 0.333;  m_coupling[3][1] = 0.000;  m_coupling[3][2] = 0.002;  m_coupling[3][3] = 0.002;

    m_parameters = par;
    m_rand = new Random(m_parameters->randomSeed);
    m_electronGrid = new CubicGrid(m_parameters->gridWidth, m_parameters->gridHeight, m_parameters->gridDepth);
    m_holeGrid = new CubicGrid(m_parameters->gridWidth, m_parameters->gridHeight, m_parameters->gridDepth);
    m_potential = new Potential(this);
    m_logger = new Logger(this);
    m_ocl = new OpenClHelper(this);
    m_source = new SourceAgent(this,0);
    m_drain = new DrainAgent(this,0);
  }

  World::~World()
  {
   delete m_rand;
   delete m_potential;
   delete m_source;
   delete m_drain;
   delete m_electronGrid;
   delete m_holeGrid;
   delete m_logger;
   delete m_ocl;
  }
}
