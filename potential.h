#ifndef POTENTIALNEW_H
#define POTENTIALNEW_H

#include <QObject>

namespace Langmuir
{

class World;
class Grid;

class Potential : public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(Potential)

public:
    Potential(World &world, QObject *parent=0);
    void setPotentialZero();
    void setPotentialLinear();
    void setPotentialTraps();
    void setPotentialFromFile(QString fileName);
    void setPotentialFromScript(QString filename);
    void updateInteractionEnergies();
    double coulombPotentialElectrons(int site);
    double coulombImageXPotentialElectrons(int site);
    double coulombPotentialHoles(int site);
    double coulombImageXPotentialHoles(int site);
    double coulombPotentialDefects(int site);
    double coulombImageXPotentialDefects(int site);
    double potentialAtSite(int site, Grid *grid = 0, bool useCoulomb = false, bool useImage = false);

private:
    World &m_world;
};

}
#endif // POTENTIALNEW_H
