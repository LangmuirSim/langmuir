#ifndef FLUXAGENT_H
#define FLUXAGENT_H

#include "agent.h"
#include "cubicgrid.h"

namespace Langmuir
{

struct SimulationParameters;

class FluxAgent : public Agent
{
public:    
    FluxAgent(Agent::Type type, World &world, Grid& grid, QObject *parent = 0);
    ~FluxAgent();

    void setPotential(double potential);
    double potential()const;

    void setRate(double rate);
    double rate()const;

    void setTries(int tries);
    int tries()const;

    void setAttempts(unsigned long int value);
    unsigned long int attempts()const;

    void setSuccesses(unsigned long int value);
    unsigned long int successes()const;

    double successProbability()const;
    double successRate()const;
    void resetCounters();

    Grid::CubeFace face()const;
    Grid& grid()const;

protected:
    void initializeSite(int site);
    void initializeSite(Grid::CubeFace cubeFace);
    virtual bool shouldTransport(int site);
    virtual double energyChange(int site);
    QString faceToLetter();
    unsigned long int m_attempts;
    unsigned long int m_successes;
    double m_probability;
    int m_tries;
    double m_potential;
    Grid &m_grid;
    Grid::CubeFace m_face;
};

}
#endif
