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
    FluxAgent(Agent::Type type, World &world, Grid& grid, QObject *parent = 0);
    ~FluxAgent();
    unsigned long int attempts()const;
    unsigned long int successes()const;
    double successProbability()const;
    double successRate()const;
    Grid::CubeFace face()const;
    void resetCounters();
    void setPotential(double potential);
    void setRate(double rate);
    void setTries(int tries);
    void setAttempts(unsigned long int value);
    void setSuccesses(unsigned long int value);

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
