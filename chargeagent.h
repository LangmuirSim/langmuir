#ifndef CHARGEAGENT_H
#define CHARGEAGENT_H

#include "agent.h"

namespace Langmuir
{
class Grid;
class SimulationParameters;
class ChargeAgent : public Agent
{
public:
    ChargeAgent(Agent::Type type, World *world, int site, QObject *parent=0);
    virtual ~ChargeAgent();
    int charge();
    void chooseFuture();
    void decideFuture();
    void completeTick();
    bool removed();
    int lifetime();
    double pathlength();
    void setOpenCLID(int id);
    int getOpenCLID();
    double coulombInteraction(int newSite);

    static void setTitles( QTextStream &stream, SimulationParameters &par );
    friend QTextStream& operator<<( QTextStream &stream, const ChargeAgent &charge );

protected:
    virtual double bindingPotential(int site)= 0;
    int m_charge;
    bool m_removed;
    int m_lifetime;
    int m_pathlength;
    Grid *m_grid;
    int m_openClID;
};

class ElectronAgent : public ChargeAgent
{
public:
    ElectronAgent(World *world, int site, QObject *parent=0);
protected:
    virtual double bindingPotential(int site);
};

class HoleAgent : public ChargeAgent
{
public:
    HoleAgent(World *world, int site, QObject *parent=0);
protected:
    virtual double bindingPotential(int site);
};
}
#endif
