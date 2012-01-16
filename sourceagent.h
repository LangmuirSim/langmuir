#ifndef SOURCEAGENT_H
#define SOURCEAGENT_H

#include "fluxagent.h"

namespace Langmuir
{
class SourceAgent : public FluxAgent
{
public:
    SourceAgent(World * world, double potential = 0.0, double rate = 0.0, int tries = 1, QObject *parent = 0);
    bool seed();
    bool tryToInject();

protected:
    virtual int chooseSite();
    virtual bool validToInject(int site)= 0;
    virtual void inject(int site)= 0;
    int randomSiteID();
    int randomNeighborSiteID();
};

class ElectronSourceAgent : public SourceAgent
{
public:
    ElectronSourceAgent(World * world, int site, double potential = 0.0, double rate = 0.0, int tries = 1, QObject *parent = 0);
    ElectronSourceAgent(World * world, Grid::CubeFace cubeFace, double potential = 0.0, double rate = 0.0, int tries = 1, QObject *parent = 0);
protected:
    virtual bool validToInject(int site);
    virtual double energyChange(int site);
    virtual void inject(int site);
};

class HoleSourceAgent : public SourceAgent
{
public:
    HoleSourceAgent(World * world, int site, double potential = 0.0, double rate = 0.0, int tries = 1, QObject *parent = 0);
    HoleSourceAgent(World * world, Grid::CubeFace cubeFace, double potential = 0.0, double rate = 0.0, int tries = 1, QObject *parent = 0);
protected:
    virtual bool validToInject(int site);
    virtual double energyChange(int site);
    virtual void inject(int site);
};

class ExcitonSourceAgent : public SourceAgent
{
public:
    ExcitonSourceAgent(World * world, double rate = 0.0, int tries = 1, QObject *parent = 0);
protected:
    virtual bool validToInject(int site);
    virtual double energyChange(int site);
    virtual int chooseSite();
    virtual void inject(int site);
};
}
#endif
