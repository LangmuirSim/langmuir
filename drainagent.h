#ifndef DRAINAGENT_H
#define DRAINAGENT_H

#include "fluxagent.h"

namespace Langmuir
{

class ChargeAgent;

class DrainAgent : public FluxAgent
{
public:
    DrainAgent(World &world, Grid &grid, QObject *parent = 0);
    virtual bool tryToAccept(ChargeAgent *charge);
};

class ElectronDrainAgent : public DrainAgent
{
public:
    ElectronDrainAgent(World &world, int site, QObject *parent = 0);
    ElectronDrainAgent(World &world, Grid::CubeFace cubeFace, QObject *parent = 0);
protected:
    virtual double energyChange(int fSite);
};

class HoleDrainAgent : public DrainAgent
{
public:
    HoleDrainAgent(World &world, int site, QObject *parent = 0);
    HoleDrainAgent(World &world, Grid::CubeFace cubeFace, QObject *parent = 0);
protected:
    virtual double energyChange(int fSite);
};

class RecombinationAgent : public DrainAgent
{
public:
    RecombinationAgent(World &world, QObject *parent = 0);
    virtual bool tryToAccept(ChargeAgent *charge);
private:
    virtual double energyChange(int fSite);
};

}
#endif
