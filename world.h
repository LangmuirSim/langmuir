#ifndef WORLD_H
#define WORLD_H
#define BOOST_DISABLE_ASSERTS

#include <QtCore>
#include <QtGui>
#include "boost/multi_array.hpp"

namespace Langmuir
{

class Grid;
class Agent;
class Random;
class Logger;
class Potential;
class FluxAgent;
class DrainAgent;
class Simulation;
class ChargeAgent;
class SourceAgent;
class OpenClHelper;
struct SimulationParameters;

class World : public QObject
{  
private:   
    Q_OBJECT
    Q_DISABLE_COPY(World)

public:
    World(SimulationParameters &par, QObject *parent = 0);
    ~World();
    Grid& electronGrid();
    Grid& holeGrid();
    Potential& potential();
    SimulationParameters& parameters();
    Random& randomNumberGenerator();
    Logger& logger();
    OpenClHelper& opencl();
    QList<SourceAgent*>& sources();
    QList<DrainAgent*>& drains();
    QList<FluxAgent*>& fluxes();
    QList<ChargeAgent*>& electrons();
    QList<ChargeAgent*>& holes();
    QList<int>& defectSiteIDs();
    QList<int>& trapSiteIDs();
    QList<double>& trapSitePotentials();
    boost::multi_array<double,3>& interactionEnergies();
    int maxElectronAgents();
    int maxHoleAgents();
    int maxChargeAgents();
    int maxCharges();
    int maxDefects();
    int maxTraps();
    int numElectronAgents();
    int numHoleAgents();
    int numChargeAgents();
    int numCharges();
    int numDefects();
    int numTraps();
    double reachedChargeAgents();
    double reachedElectronAgents();
    double reachedHoleAgents();
    double percentHoleAgents();
    double percentElectronAgents();
    void saveCheckpointFile(const QString &name = "%path/%stub.bin");

private:
    QList<SourceAgent*> m_sources;
    QList<DrainAgent*> m_drains;
    QList<FluxAgent*> m_fluxAgents;
    Grid *m_electronGrid;
    Grid *m_holeGrid;
    Random *m_rand;
    Potential *m_potential;
    SimulationParameters *m_parameters;
    Logger *m_logger;
    OpenClHelper *m_ocl;
    QList<ChargeAgent*> m_electrons;
    QList<ChargeAgent*> m_holes;
    QList<int> m_defectSiteIDs;
    QList<int> m_trapSiteIDs;
    QList<double> m_trapSitePotentials;
    boost::multi_array<double,3> m_interactionEnergies;
    int m_maxElectrons;
    int m_maxHoles;
    int m_maxDefects;
    int m_maxTraps;
    void placeDefects(const QList<int>& siteIDs = QList<int>());
    void placeElectrons(const QList<int>& siteIDs = QList<int>());
    void placeHoles(const QList<int>& siteIDs = QList<int>());
    void initialize();
    void checkDataStream(QDataStream& stream, const QString& message = "");
};

}
#endif // WORLD_H
