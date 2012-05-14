#ifndef SIMULATION_H
#define SIMULATION_H

#include <QObject>

namespace Langmuir
{

class World;
class Grid;
class Agent;
class Potential;
class DrainAgent;
class SourceAgent;
class ChargeAgent;
struct SimulationParameters;

class Simulation : public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(Simulation)

public:
    Simulation(World &world, QObject *parent = 0);
    virtual ~Simulation();
    virtual void performIterations(int nIterations);
    void equilibrated();

protected:
    void performRecombinations();
    void performInjections();
    void nextTick();
    static void chargeAgentCoulombInteractionQtConcurrentCPU(ChargeAgent * chargeAgent);
    static void chargeAgentCoulombInteractionQtConcurrentGPU(ChargeAgent * chargeAgent);
    World &m_world;
};

}

#endif
