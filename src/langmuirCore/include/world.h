#ifndef WORLD_H
#define WORLD_H
#define BOOST_DISABLE_ASSERTS

#include <QtCore>
#include <QtGui>

#ifndef Q_MOC_RUN
#pragma GCC diagnostic push
#pragma GCC system_header
#include "boost/multi_array.hpp"
#pragma GCC diagnostic pop
#endif

namespace Langmuir
{

class Grid;
class Agent;
class Random;
class Logger;
class KeyValueParser;
class Potential;
class FluxAgent;
class DrainAgent;
class HoleDrainAgent;
class ElectronDrainAgent;
class RecombinationAgent;
class Simulation;
class ChargeAgent;
class SourceAgent;
class HoleSourceAgent;
class ExcitonSourceAgent;
class ElectronSourceAgent;
class CheckPointer;
class OpenClHelper;
struct SimulationParameters;

/**
 * @brief A class to hold all objects in a simulation
 */
class World : public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(World)

public:
    /**
     * @brief create a world to simulate in
     * @param fileName the input file name
     * @param parent QObject this belongs to
     *
     * Calls the initialize() function.
     */
    World(const QString& fileName, QObject *parent = 0);

    /**
     * @brief destroys the entire World, and everything in it...including you.
     */
    ~World();

    /**
     * @brief get the KeyValueParser, used for parsing input files.
     */
    KeyValueParser& keyValueParser();

    /**
     * @brief get the CheckPointer, used for reading and writing input files.
     */
    CheckPointer& checkPointer();

    /**
     * @brief get the Grid used, used for holding ElectronAgents
     */
    Grid& electronGrid();

    /**
     * @brief get the hole Grid, used for holding HoleAgents
     */
    Grid& holeGrid();

    /**
     * @brief get the Potential, a calculator used for...calculating the potential.
     */
    Potential& potential();

    /**
     * @brief get the SimulationParameters, a struct used for holding simulation parameters.
     */
    SimulationParameters& parameters();

    /**
     * @brief get the Random, used for creating random numbers
     */
    Random& randomNumberGenerator();

    /**
     * @brief get the Logger, used for writing output
     */
    Logger& logger();

    /**
     * @brief get the OpenClHelper, used for calculating Coulomb interactions with a Graphics Card
     */
    OpenClHelper& opencl();

    /**
     * @brief get a list of all SourceAgents
     */
    QList<SourceAgent*>& sources();

    /**
     * @brief get a list of all ElectronSourceAgents
     */
    QList<SourceAgent*>& eSources();

    /**
     * @brief get a list of all ElectronSourceAgents
     */
    QList<SourceAgent*>& hSources();

    /**
     * @brief get a list of all ElectronSourceAgents
     */
    QList<SourceAgent*>& xSources();

    /**
     * @brief get a list of all DrainAgents
     */
    QList<DrainAgent*>& drains();

    /**
     * @brief get a list of all ElectronSourceAgents
     */
    QList<DrainAgent*>& eDrains();

    /**
     * @brief get a list of all ElectronSourceAgents
     */
    QList<DrainAgent*>& hDrains();

    /**
     * @brief get a list of all ElectronSourceAgents
     */
    QList<DrainAgent*>& xDrains();

    /**
     * @brief get a list of all FluxAgents
     */
    QList<FluxAgent*>& fluxes();

    /**
     * @brief get the right ElectronSourceAgent
     */
    ElectronSourceAgent& electronSourceAgentRight();

    /**
     * @brief get the left ElectronSourceAgent
     */
    ElectronSourceAgent& electronSourceAgentLeft();

    /**
     * @brief get the right HoleSourceAgent
     */
    HoleSourceAgent& holeSourceAgentRight();

    /**
     * @brief get the left HoleSourceAgent
     */
    HoleSourceAgent& holeSourceAgentLeft();

    /**
     * @brief get the RecombinationAgent
     */
    ExcitonSourceAgent& excitonSourceAgent();

    /**
     * @brief get the right ElectronDrainAgent
     */
    ElectronDrainAgent& electronDrainAgentRight();

    /**
     * @brief get the left ElectronDrainAgent
     */
    ElectronDrainAgent& electronDrainAgentLeft();

    /**
     * @brief get the right HoleDrainAgent
     */
    HoleDrainAgent& holeDrainAgentRight();

    /**
     * @brief get the left HoleDrainAgent
     */
    HoleDrainAgent& holeDrainAgentLeft();

    /**
     * @brief get the RecombinationAgent
     */
    RecombinationAgent& recombinationAgent();

    /**
     * @brief get a list of all ElectronAgents
     */
    QList<ChargeAgent*>& electrons();

    /**
     * @brief get a list of all HoleAgents
     */
    QList<ChargeAgent*>& holes();

    /**
     * @brief get a list of all defect sites
     */
    QList<int>& defectSiteIDs();

    /**
     * @brief get a list of all trap sites
     */
    QList<int>& trapSiteIDs();

    /**
     * @brief get a list of all trap potentials
     */
    QList<double>& trapSitePotentials();

    /**
     * @brief get the array of precomputed r-squared values
     */
    boost::multi_array<double,3>& R1();

    /**
     * @brief get the array of precomputed r values
     */
    boost::multi_array<double,3>& R2();

    /**
     * @brief get the array of precomputed inverse-r values
     */
    boost::multi_array<double,3>& iR();

    /**
     * @brief get the array of precomputed erf(r/(sqrt(2)*sigma))
     */
    boost::multi_array<double,3>& eR();

    /**
     * @brief get the self interactions
     */
    boost::multi_array<double, 3>& sI();

    /**
     * @brief get the coupling constants
     */
    boost::multi_array<double,3>& couplingConstants();

    /**
     * @brief get the max number of ElectronAgents allowed
     */
    int maxElectronAgents();

    /**
     * @brief get the max number of HoleAgents allowed
     */
    int maxHoleAgents();

    /**
     * @brief get the max number of ChargeAgents allowed
     */
    int maxChargeAgents();

    /**
     * @brief get the max number of ChargeAgents & charged defects
     */
    int maxChargeAgentsAndChargedDefects();

    /**
     * @brief get the max number of Defects allowed
     */
    int maxDefects();

    /**
     * @brief get the max number of Traps allowed
     */
    int maxTraps();

    /**
     * @brief get the current number of ElectronAgents
     */
    int numElectronAgents();

    /**
     * @brief get the current number of HoleAgents
     */
    int numHoleAgents();

    /**
     * @brief get the current number of ChargeAgents
     */
    int numChargeAgents();

    /**
     * @brief The number of electrons - holes
     */
    int electronsMinusHoles();

    /**
     * @brief The number of holes - electrons
     */
    int holesMinusElectrons();

    /**
     * @brief true when electrons and holes are balanced
     */
    bool chargesAreBalanced();

    /**
     * @brief get the current number of ChargeAgents & charged defects
     */
    int numChargeAgentsAndChargedDefects();

    /**
     * @brief get the current number of Defects
     */
    int numDefects();

    /**
     * @brief get the current number of Traps
     */
    int numTraps();

    /**
     * @brief get the percent of ChargeAgents reached, of the max allowed
     */
    double reachedChargeAgents();

    /**
     * @brief get the percent of ElectronAgents reached, of the max allowed
     */
    double reachedElectronAgents();

    /**
     * @brief get the percent of HoleAgents reached, of the max allowed
     */
    double reachedHoleAgents();

    /**
     * @brief get the percent of HoleAgents reached, of the total grid volume
     */
    double percentHoleAgents();

    /**
     * @brief get the percent of ElectronAgents reached, of the total grid volume
     */
    double percentElectronAgents();

    /**
     * @brief check if the maximum number of electrons has been reached
     */
    bool atMaxElectrons();

    /**
     * @brief check if the maximum number of holes has been reached
     */
    bool atMaxHoles();

    /**
     * @brief check if the maximum number of charges has been reached
     */
    bool atMaxCharges();

private:
    /**
     * @brief pointer to KeyValueParser, used for parsing key=value pairs
     */
    KeyValueParser *m_keyValueParser;

    /**
     * @brief pointer to CheckPointer, used for reading/writing input(checkpoint) files
     */
    CheckPointer *m_checkPointer;

    /**
     * @brief list of SourceAgents
     */
    QList<SourceAgent*> m_sources;

    /**
     * @brief list of ElectronSourceAgents
     */
    QList<SourceAgent*> m_eSources;

    /**
     * @brief list of HoleSourceAgents
     */
    QList<SourceAgent*> m_hSources;

    /**
     * @brief list of ExcitonSourceAgents
     */
    QList<SourceAgent*> m_xSources;

    /**
     * @brief list of DrainAgents
     */
    QList<DrainAgent*> m_drains;

    /**
     * @brief list of ElectronSourceAgents
     */
    QList<DrainAgent*> m_eDrains;

    /**
     * @brief list of HoleSourceAgents
     */
    QList<DrainAgent*> m_hDrains;

    /**
     * @brief list of ExcitonSourceAgents
     */
    QList<DrainAgent*> m_xDrains;

    /**
     * @brief list of all FluxAgents, such as SoureAgents, DrainAgents, etc.
     */
    QList<FluxAgent*> m_fluxAgents;

    /**
     * @brief pointer to right ElectronDrainAgent
     */
    ElectronSourceAgent *m_electronSourceAgentRight;

    /**
     * @brief pointer to left ElectronDrainAgent
     */
    ElectronSourceAgent *m_electronSourceAgentLeft;

    /**
     * @brief pointer to right HoleDrainAgent
     */
    HoleSourceAgent *m_holeSourceAgentRight;

    /**
     * @brief pointer to left HoleDrainAgent
     */
    HoleSourceAgent *m_holeSourceAgentLeft;

    /**
     * @brief pointer to ExcitonSourceAgent, used for injecting Excitons
     */
    ExcitonSourceAgent *m_excitonSourceAgent;

    /**
     * @brief pointer to right ElectronDrainAgent
     */
    ElectronDrainAgent *m_electronDrainAgentRight;

    /**
     * @brief pointer to left ElectronDrainAgent
     */
    ElectronDrainAgent *m_electronDrainAgentLeft;

    /**
     * @brief pointer to right HoleDrainAgent
     */
    HoleDrainAgent *m_holeDrainAgentRight;

    /**
     * @brief pointer to left HoleDrainAgent
     */
    HoleDrainAgent *m_holeDrainAgentLeft;

    /**
     * @brief pointer to electron/hole RecombinationAgent, used for removing Excitons
     */
    RecombinationAgent *m_recombinationAgent;

    /**
     * @brief pointer to electron Grid, used for keeping track of ElectronAgents
     */
    Grid *m_electronGrid;

    /**
     * @brief pointer to hole Grid, used for keeping track of HoleAgents
     */
    Grid *m_holeGrid;

    /**
     * @brief pointer to Random, used for generating random numbers
     */
    Random *m_rand;

    /**
     * @brief pointer to Potential, used for calculating the potential
     */
    Potential *m_potential;

    /**
     * @brief pointer to SimulationParameters
     */
    SimulationParameters *m_parameters;

    /**
     * @brief pointer to Logger, used for output
     */
    Logger *m_logger;

    /**
     * @brief pointer to OpenClHelper, used for Graphics Card calculations
     */
    OpenClHelper *m_ocl;

    /**
     * @brief list of electrons
     */
    QList<ChargeAgent*> m_electrons;

    /**
     * @brief list of holes
     */
    QList<ChargeAgent*> m_holes;

    /**
     * @brief list of defect sites
     */
    QList<int> m_defectSiteIDs;

    /**
     * @brief list of trap sites
     */
    QList<int> m_trapSiteIDs;

    /**
     * @brief list of trap potentials
     */
    QList<double> m_trapSitePotentials;

    /**
     * @brief array of precomputed r-squared values
     *
     * This array is indexed by dx, dy, dz values, and r is in grid-units
     */
    boost::multi_array<double,3> m_R2;

    /**
     * @brief array of precomputed r values
     *
     * This array is indexed by dx, dy, dz values, and r is in grid-units
     */
    boost::multi_array<double,3> m_R1;

    /**
     * @brief array of precomputed inverse-r values
     *
     * This array is indexed by dx, dy, dz values, and r is in grid-units
     */
    boost::multi_array<double,3> m_iR;

    /**
     * @brief array of precomputed erf(r/(s*sqrt(2)) values
     *
     * This array is indexed by dx, dy, dz values, and r is in grid-units
     */
    boost::multi_array<double,3> m_eR;

    /**
     * @brief self interaction, which is 1/(4 pi e e0 r), with r=1 grid unit
     * When a charge at it's future site interacts with other charges at their
     * current site, the charge will interact with it's own current site.  So,
     * this value needs to be subtracted off.
     */
    boost::multi_array<double, 3> m_sI;

    /**
     * @brief array of coupling constants
     *
     * This array is indexed by dx, dy, dz values.
     */
    boost::multi_array<double,3> m_couplingConstants;

    /**
     * @brief max number of electrons
     */
    int m_maxElectrons;

    /**
     * @brief max number of holes
     */
    int m_maxHoles;

    /**
     * @brief max number of defects
     */
    int m_maxDefects;

    /**
     * @brief max number of traps
     */
    int m_maxTraps;

    /**
     * @brief places defects
     * @param siteIDs a list of defect site ids
     *
     * Places carriers according to the site ids passed.  If
     * more need placing (according to SimulationParameters::seedCharges),
     * then they are placed randomly.
     */
    void placeDefects(const QList<int>& siteIDs = QList<int>());

    /**
     * @brief places electrons
     * @param siteIDs a list of electron site ids
     *
     * Places carriers according to the site ids passed.  If
     * more need placing (according to SimulationParameters::seedCharges),
     * then they are placed randomly.
     */
    void placeElectrons(const QList<int>& siteIDs = QList<int>());

    /**
     * @brief places holes
     * @param siteIDs a list of hole site ids
     *
     * Places carriers according to the site ids passed.  If
     * more need placing (according to SimulationParameters::seedCharges),
     * then they are placed randomly.
     */
    void placeHoles(const QList<int>& siteIDs = QList<int>());

    /**
     * @brief create SourceAgents
     */
    void createSources();

    /**
     * @brief create DrainAgents
     */
    void createDrains();

    /**
     * @brief set attempts / successes for sources / drains
     */
    void setFluxInfo(const QList<quint64> &fluxInfo);

    /**
     * @brief initialize all objects
     * @param fileName input file name
     *
     * A very long, though not all that complicated function that creates
     * all the simulation objects.  Best to read through it in the source
     * code.
     */
    void initialize(const QString& fileName = "");
};

}
#endif // WORLD_H
