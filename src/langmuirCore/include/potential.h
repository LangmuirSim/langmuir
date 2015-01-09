#ifndef POTENTIALNEW_H
#define POTENTIALNEW_H
#define BOOST_DISABLE_ASSERTS

#include <QObject>

#ifndef Q_MOC_RUN

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#pragma GCC diagnostic push
#pragma GCC system_header
#endif

#include "boost/multi_array.hpp"

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#pragma GCC diagnostic pop
#endif

#endif

namespace LangmuirCore
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
     * @brief pre-calculates r2, r, and 1/r
     */
    void precalculateArrays();

    /**
     * @brief pre-calculates coupling constants
     */
    void updateCouplingConstants();

    /**
     * @brief calculates Coulomb potential from electrons at specific grid site
     * @param site_i the site of interest
     */
    double coulombE(int site_i);

    /**
     * @brief calculates Coulomb image-potential from electrons at specific grid site
     * @param site_i the site of interest
     */
    double coulombImageE(int site_i);

    /**
     * @brief calculates Coulomb potential from electrons at specific grid site, assuming gaussians
     * @param site_i the site of interest
     */
    double gaussE(int site_i);

    /**
     * @brief calculates Coulomb image-potential from electrons at specific grid site, assuming gaussians
     * @param site_i the site of interest
     */
    double gaussImageE(int site_i);

    /**
     * @brief calculates Coulomb potential from holes at specific grid site
     * @param site_i the site of interest
     */
    double coulombH(int site_i);

    /**
     * @brief calculates Coulomb image-potential from holes at specific grid site
     * @param site_i the site of interest
     */
    double coulombImageH(int site_i);

    /**
     * @brief calculates Coulomb potential from holes at specific grid site, assuming gaussians
     * @param site the site of interest
     */
    double gaussH(int site);

    /**
     * @brief calculates Coulomb image-potential from holes at specific grid site, assuming gaussians
     * @param site the site of interest
     */
    double gaussImageH(int site);

    /**
     * @brief calculates Coulomb potential from charged defects at specific grid site
     * @param site_i the site of interest
     */
    double coulombD(int site_i);

    /**
     * @brief calculates Coulomb image-potential from charged defects at specific grid site
     * @param site_i the site of interest
     */
    double coulombImageD(int site_i);

    /**
     * @brief calculates Coulomb potential from charged defects at specific grid site, assuming gaussians
     * @param site_i the site of interest
     */
    double gaussD(int site_i);

    /**
     * @brief calculates Coulomb image-potential from charged defects at specific grid site, assuming gaussians
     * @param site_i the site of interest
     */
    double gaussImageD(int site_i);

private:
    /**
     * @brief reference to the World
     */
    World &m_world;
};

}
#endif // POTENTIALNEW_H
