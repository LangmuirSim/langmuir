#ifndef CHARGEAGENT_H
#define CHARGEAGENT_H

#include "agent.h"

namespace Langmuir
{

class Grid;
struct SimulationParameters;

class ChargeAgent : public Agent
{
public:
    ChargeAgent(Agent::Type getType, World &world, Grid &grid, int site, QObject *parent=0);
    virtual ~ChargeAgent();
    int charge();
    void chooseFuture();
    void decideFuture();
    void completeTick();
    bool removed();
    int lifetime();
    int pathlength();
    void setOpenCLID(int id);
    int getOpenCLID();
    double coulombInteraction();
    void coulombCPU();
    void coulombGPU();
    Grid& getGrid();
    void setRemoved(const bool &status = true);
    virtual Agent::Type otherType() = 0;
    virtual Grid& otherGrid() = 0;

protected:
    virtual double bindingPotential(int site)= 0;
    int m_charge;
    bool m_removed;
    int m_lifetime;
    int m_pathlength;
    Grid &m_grid;
    int m_openClID;
    double m_de;
};

class ElectronAgent : public ChargeAgent
{
public:
    ElectronAgent(World &world, int site, QObject *parent=0);
protected:
    virtual double bindingPotential(int site);
    virtual Agent::Type otherType();
    virtual Grid& otherGrid();
};

class HoleAgent : public ChargeAgent
{
public:
    HoleAgent(World &world, int site, QObject *parent=0);
protected:
    virtual double bindingPotential(int site);
    virtual Agent::Type otherType();
    virtual Grid& otherGrid();
};

}
#endif
