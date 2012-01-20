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

World::World(SimulationParameters *par, QObject *parent) : QObject(parent)
{
    m_parameters = par;
    m_rand = new Random(m_parameters->randomSeed);
    m_electronGrid = new Grid(m_parameters->gridX, m_parameters->gridY, m_parameters->gridZ, this);
    m_holeGrid = new Grid(m_parameters->gridX, m_parameters->gridY, m_parameters->gridZ, this);
    m_potential = new Potential(this, this);
    //m_logger = new Logger(this, this);
    m_ocl = new OpenClHelper(this, this);

    //transistor sources
    //m_sources.push_back( new ElectronSourceAgent(this, Grid::Left, 0, 0.9, 1, this) );

    //transistor drains
    //m_drains.push_back( new ElectronDrainAgent(this, Grid::Right, 0, 0.9, 1, this) );

    //solar cell sources
    m_sources.push_back(new ExcitonSourceAgent(this, 0.9, 1, this));

    //solar cell drains
    m_drains.push_back(new ElectronDrainAgent(this, Grid::Left, 0, 0.9, 1, this));
    m_drains.push_back(new ElectronDrainAgent(this, Grid::Right, 0, 0.9, 1, this));
    m_drains.push_back(new HoleDrainAgent(this, Grid::Left, 0, 0.9, 1, this));
    m_drains.push_back(new HoleDrainAgent(this, Grid::Right, 0, 0.9, 1, this));

    QString string;
    QTextStream stream(&string);
    stream << this->metaObject()->className() << "(" << this << ")";
    setObjectName(string);
}

World::~World()
{
    for(int i = 0; i < m_sources.size(); i++)
    {
        delete m_sources[i];
    }
    m_sources.clear();

    for(int i = 0; i < m_drains.size(); i++)
    {
        delete m_drains[i];
    }
    m_drains.clear();

    for(int i = 0; i < m_electrons.size(); i++)
    {
        delete m_electrons[i];
    }
    m_electrons.clear();

    for(int i = 0; i < m_holes.size(); i++)
    {
        delete m_holes[i];
    }
    m_holes.clear();

    delete m_rand;
    delete m_potential;
    delete m_electronGrid;
    delete m_holeGrid;
    //delete m_logger;
    delete m_ocl;
}
}
