#ifndef DRAINAGENT_H
#define DRAINAGENT_H

#include "fluxagent.h"

namespace Langmuir
{
class ChargeAgent;
class DrainAgent : public FluxAgent
{
public:
    DrainAgent(World * world, double potential = 0.0, double rate = 0.0, int tries = 1, QObject *parent = 0);
    bool tryToAccept(ChargeAgent *charge);

protected:
};

class ElectronDrainAgent : public DrainAgent
{
public:
    ElectronDrainAgent(World * world, int site, double potential = 0.0, double rate = 0.0, int tries = 1, QObject *parent = 0);
    ElectronDrainAgent(World * world, Grid::CubeFace cubeFace, double potential = 0.0, double rate = 0.0, int tries = 1, QObject *parent = 0);
protected:
    virtual double energyChange(int fSite);
};

class HoleDrainAgent : public DrainAgent
{
public:
    HoleDrainAgent(World * world, int site, double potential = 0.0, double rate = 0.0, int tries = 1, QObject *parent = 0);
    HoleDrainAgent(World * world, Grid::CubeFace cubeFace, double potential = 0.0, double rate = 0.0, int tries = 1, QObject *parent = 0);
protected:
    virtual double energyChange(int fSite);
};
}
#endif
