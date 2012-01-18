#ifndef FLUXAGENT_H
#define FLUXAGENT_H

#include "agent.h"
#include "cubicgrid.h"

namespace Langmuir
{
class SimulationParameters;
class FluxAgent : public Agent
{
public:
    FluxAgent(Agent::Type type, World * world, double potential = 0.0, double rate = 0.0, int tries = 1, QObject *parent = 0);
    ~FluxAgent();
    int maxCarriers()const;
    int maxElectrons()const;
    int maxHoles()const;
    int carrierCount()const;
    int electronCount()const;
    int holeCount()const;
    double percentCarriers()const;
    double percentElectrons()const;
    double percentHoles()const;
    unsigned long int attempts()const;
    unsigned long int successes()const;
    double successRate()const;
    void resetCounters();

    static void titleString( QTextStream &stream, SimulationParameters &par );
    friend QTextStream& operator<<( QTextStream &stream, const FluxAgent &flux );

protected:
    void initializeSite(int site);
    void initializeSite(Grid::CubeFace cubeFace);
    virtual bool shouldTransport(int site);
    virtual double energyChange(int site);
    static int m_maxHoles;
    static int m_maxElectrons;
    unsigned long int m_attempts;
    unsigned long int m_successes;
    double m_probability;
    int m_tries;
    double m_potential;
    Grid *m_grid;
    Grid::CubeFace m_face;
};

}
#endif
