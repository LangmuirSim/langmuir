#ifndef POTENTIALNEW_H
#define POTENTIALNEW_H

#include <QObject>

namespace Langmuir
{

class World;
class Grid;

/**
 * @brief A class to calculate the potential
 */
class Potential : public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(Potential)

public:
    /**
     * @brief Potential Create the potential
     * @param world reference to the World
     * @param parent QObject this belongs to
     */
    Potential(World &world, QObject *parent=0);

    /**
     * @brief sets the value of the potential to zero at every grid site
     */
    void setPotentialZero();

    /**
     * @brief Adds a linear potential calcualted from voltage.left and voltage.right along the x-direction
     */
    void setPotentialLinear();

    /**
     * @brief Adds a linear potential calculated from slope.z along the z-direction
     */
    void setPotentialGate();

    /**
     * @brief Adds shifts to the potential at the various sites
     * @param trapIDs list of site ids
     * @param trapPotentials list of shifts
     */
    void setPotentialTraps(
        const QList<int>& trapIDs = QList<int>(),
        const QList<double>& trapPotentials = QList<double>()
        );

    /**
     * @brief pre-calculates e/(4 pi epsilon epsilon_0 r) in units of eV * nm
     */
    void updateInteractionEnergies();

    /**
     * @brief pre-calculates coupling constants
     */
    void updateCouplingConstants();

    /**
     * @brief calculates Coulomb potential from electrons at specific grid site
     * @param site the site of interest
     */
    double coulombPotentialElectrons(int site);

    /**
     * @brief calculates Coulomb image-potential from electrons at specific grid site
     * @param site the site of interest
     */
    double coulombImageXPotentialElectrons(int site);

    /**
     * @brief calculates Coulomb potential from holes at specific grid site
     * @param site the site of interest
     */
    double coulombPotentialHoles(int site);

    /**
     * @brief calculates Coulomb image-potential from holes at specific grid site
     * @param site the site of interest
     */
    double coulombImageXPotentialHoles(int site);

    /**
     * @brief calculates Coulomb potential from charged defects at specific grid site
     * @param site the site of interest
     */
    double coulombPotentialDefects(int site);

    /**
     * @brief calculates Coulomb image-potential from charged defects at specific grid site
     * @param site the site of interest
     */
    double coulombImageXPotentialDefects(int site);

    /**
     * @brief calculate the potential at a specific site, including all contributions
     * @param site the site of interest
     * @param grid the grid
     * @param useCoulomb include coulomb interactions
     * @param useImage include coulomb image-interactions
     * @return
     */
    double potentialAtSite(int site,
                           Grid *grid = 0,
                           bool useCoulomb = false,
                           bool useImage = false);
private:
    /**
     * @brief reference to the World
     */
    World &m_world;
};

}
#endif // POTENTIALNEW_H
