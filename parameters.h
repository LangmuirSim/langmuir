#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <QDateTime>
#include <QFileInfo>
#include <QDebug>
#include <cmath>
#include <QDir>

namespace Langmuir
{

struct SimulationSiteInfo
{
    //! a list of current electron site IDs
    QList<qint32> electrons;

    //! a list of current holes site IDs
    QList<qint32> holes;

    //! a list of current defects site IDs
    QList<qint32> defects;

    //! a list of current traps site IDs
    QList<qint32> traps;

    //! a list of current traps site IDs
    QList<qreal> potentials;

    //! clear the lists
    void clear()
    {
        electrons.clear();
        holes.clear();
        defects.clear();
        traps.clear();
        potentials.clear();
    }
};

struct SimulationParameters
{
    //! name of the input parameters file
    QString checkFile;

    //! tells Langmuir how to set up the Sources and Drains: (\b\c "transistor", \b\c "solarcell")
    QString simulationType;

    //! seed the random number generator, if negative, uses the current time (making seperate runs random)
    quint64 randomSeed;

    //! the number of sites per layer, at least one
    qint32 gridZ;

    //! the number of sites along the device width, at least one
    qint32 gridY;

    //! the number of sites along the device length, at least one
    qint32 gridX;

    //! turn on Coulomb interactions between ChargeAgents
    bool coulombCarriers;

    //! the charge of defect sites
    qint32 defectsCharge;

    //! output trajectory file every interationsPrint
    qint32 outputXyz;

    //! output carrier lifetime and pathlength when they are deleted
    bool outputIdsOnDelete;

    //! output coulomb energy for the entire grid every iterationsPrint
    qint32 outputCoulomb;

    //! output grid potential at the start of the simulation, includes the trap potential
    bool outputPotential;

    //! if false, produce no output (useful for LangmuirView)
    bool outputIsOn;

    //! if true, backup files when they exist
    bool outputBackup;

    //! if true, append to data files; if false, overwrite files
    bool outputAppend;

    //! output images of defects
    bool imageDefects;

    //! output images of defects
    bool imageTraps;

    //! output images of carriers (if n < 0, only at the end; if np == 0, never; if n > 0, every n * iterations.print steps)
    qint32 imageCarriers;

    //! if Langmuir, how often to output; if LangmuirView, how many steps between rendering
    qint32 iterationsPrint;

    //! number of simulation steps after equilibration
    qint32 iterationsReal;

    //! number of significant figures used for doubles in output
    qint32 outputPrecision;

    //! width of columns in output, ignored in certain files, like trajectory files
    qint32 outputWidth;

     //! the output directory, defaults to the current working directory
    QString outputPath;

    //! the stub to use when naming output files
    QString outputStub;

    //! the percent of the grid that is reserved for electrons, between 0 and 1
    qreal electronPercentage;

    //! the percent of the grid that is reserved for holes, between 0 and 1
    qreal holePercentage;

    //! if true, place charges randomly before the simulation starts
    qreal seedCharges;

    //! the percent of the grid that is reserved for electrons, between 0 and 1
    qreal defectPercentage;

    //! the percent of the grid that is reserved for traps, between 0 and 1
    qreal trapPercentage;

    //! the potential of traps
    qreal trapPotential;

    //! the standard deviation of trap sites
    qreal gaussianStdev;

    //! the percent of the traps to be placed and grown upon to form islands
    qreal seedPercentage;

    //! the potential on the right side of the grid, used in setting up an electric field
    qreal voltageRight;

    //! the potential on the left side of the grid, used in setting up an electric field
    qreal voltageLeft;

    //! the temperature used in the boltzmann factor
    qreal temperatureKelvin;

    //! the rate at which all sources inject charges
    qreal sourceRate;

    //! the rate at which all drains accept chargew
    qreal drainRate;

    //! if true, try to use OpenCL to speed up Coulomb interaction calculations
    bool useOpenCL;

    //! the x size of OpenCL 3DRange kernel work groups - only needed if using SimulationParameters::outputCoulomb
    qint32 workX;

    //! the y size of OpenCL 3DRange kernel work groups - only needed if using SimulationParameters::outputCoulomb
    qint32 workY;

    //! the z size of OpenCL 3DRange kernel work groups - only needed if using SimulationParameters::outputCoulomb
    qint32 workZ;

    //! the size of OpenCL 1DRange kernel work groups
    qint32 workSize;

    //! physical constant, the boltzmann constant
    qreal boltzmannConstant;

    //! physical constant, the dielectic constant
    qreal dielectricConstant;

    //! physical constant, the elementary charge
    qreal elementaryCharge;

    //! physical constant, the permittivity of free spece
    qreal permittivitySpace;

    //! size constant, the size associated with grid sites (~1nm)
    qreal gridFactor;

    //! the cut off for Coulomb interations
    qint32 electrostaticCutoff;

    //! a compilation of physical constants
    qreal electrostaticPrefactor;

    //! a compilation of physical constants
    qreal inverseKT;

    //! if true, OpenCL can be used on this platform
    bool okCL;

    //! the current step of the simulation
    quint32 currentStep;

    //! the time this simulation started
    QDateTime simulationStart;

    SimulationParameters() :

        checkFile              (""),
        simulationType         ("transistor"),
        randomSeed             (0),

        gridZ                  (1),
        gridY                  (128),
        gridX                  (128),

        coulombCarriers        (false),
        defectsCharge          (0),

        outputXyz              (0),
        outputIdsOnDelete      (false),
        outputCoulomb          (0),
        outputPotential        (false),
        outputIsOn             (true),
        outputBackup           (true),
        outputAppend           (true),

        imageDefects           (false),
        imageTraps             (false),
        imageCarriers          (0),

        iterationsPrint        (10),
        iterationsReal         (1000),
        outputPrecision        (15),
        outputWidth            (23),
        outputPath             (QDir::currentPath()),
        outputStub             ("out"),

        electronPercentage     (0.01),
        holePercentage         (0.00),
        seedCharges            (0.00),
        defectPercentage       (0.00),
        trapPercentage         (0.00),
        trapPotential          (0.10),
        gaussianStdev          (0.00),
        seedPercentage         (1.0),

        voltageRight           (0.00),
        voltageLeft            (0.00),
        temperatureKelvin      (300.0),

        sourceRate             (0.90),
        drainRate              (0.90),

        useOpenCL              (false),
        workX                  (4),
        workY                  (4),
        workZ                  (4),
        workSize               (256),

        boltzmannConstant      (1.3806504e-23),
        dielectricConstant     (3.5),
        elementaryCharge       (1.60217646e-19),
        permittivitySpace      (8.854187817e-12),
        gridFactor             (1e-9),
        electrostaticCutoff    (50),
        electrostaticPrefactor (0),
        inverseKT              (0),
        okCL                   (false),
        currentStep            (0),
        simulationStart        (QDateTime::currentDateTime())
    {
    }

};

//! sets parameters that depend upon other parameters
inline void setCalculatedValues(SimulationParameters& par)
{
    par.electrostaticPrefactor  = par.elementaryCharge /(4.0 * M_PI * par.dielectricConstant * par.permittivitySpace * par.gridFactor);
    par.inverseKT = par.elementaryCharge /(par.boltzmannConstant * par.temperatureKelvin);
    if (par.outputWidth < 8 ) { par.outputWidth = 8; }
    if (par.outputWidth < (par.outputPrecision + 8)) { par.outputWidth = par.outputPrecision + 8; }
}

//! check the parameters, making sure they are valid
inline void check(SimulationParameters& par)
{
    setCalculatedValues(par);
    // simulation type
    if (!(QStringList()<<"transistor"<<"solarcell").contains(par.simulationType))
    {
        qFatal("simulation.type(%s) must be transistor or solarcell",qPrintable(par.simulationType));
    }

    // grid size
    if (par.gridZ < 1)
    {
        qFatal("grid.x(%d) >= 1",par.gridX);
    }
    if (par.gridY < 1)
    {
        qFatal("grid.y(%d) >= 1",par.gridY);
    }
    if (par.gridX < 1)
    {
        qFatal("grid.z(%d) >= 1",par.gridZ);
    }

    // output
    if (par.iterationsPrint <= 0 )
    {
        qFatal("iterations.print(%d) <= 0",par.iterationsPrint);
    }
    if (par.iterationsReal < 0 )
    {
        qFatal("iterations.real(%d) < 0",par.iterationsReal);
    }
    if (par.iterationsReal % par.iterationsPrint != 0 )
    {
        qFatal("iterations.real(%d) %% iterations.print(%d) != 0",par.iterationsReal,par.iterationsPrint);
    }

    // percentages
    if (par.electronPercentage < 0.0 || par.electronPercentage > 1.0 )
    {
        qFatal("electron.pecentage(%f) < 0 || > 1.0",par.electronPercentage);
    }
    if (par.holePercentage     < 0.0 ||par. holePercentage     > 1.0 )
    {
        qFatal("hole.pecentage(%f) < 0 || > 1.0",par.holePercentage);
    }
    if (par.defectPercentage   < 0.0 || par.defectPercentage   > 1.0 )
    {
        qFatal("defect.pecentage(%f) < 0 || > 1.0",par.defectPercentage);
    }
    if (par.trapPercentage     < 0.0 || par.trapPercentage     > 1.0 )
    {
        qFatal("trap.pecentage(%f) < 0 || > 1.0",par.trapPercentage);
    }
    if (par.seedPercentage     < 0.0 || par.seedPercentage     > 1.0 )
    {
        qFatal("seed.pecentage(%f) < 0 || > 1.0",par.seedPercentage);
    }
    if (par.defectPercentage > 1.00 - par.trapPercentage)
    {
        qFatal("trap.percentage(%f) > 1.0 - trap.percentage(%f)",par.defectPercentage,par.trapPercentage);
    }
    if (par.sourceRate < 0.0 || par.sourceRate > 1.0 )
    {
        qFatal("source.rate(%f) < 0 || > 1.0",par.sourceRate);
    }
    if (par.drainRate < 0.0 || par.drainRate > 1.0 )
    {
        qFatal("drain.rate(%f) < 0 || > 1.0",par.drainRate);
    }
    if (par.seedCharges < 0.0 || par.seedCharges > 1.0 )
    {
        qFatal("seed.charges(%f) < 0 || > 1.0",par.seedCharges);
    }

    // other
    if (par.gaussianStdev < 0.00)
    {
        qFatal("gaussian.stdev < 0.0");
    }
    if (par.temperatureKelvin < 0.0)
    {
        qFatal("temperature.kelvin < 0.0");
    }

    if (par.defectsCharge != 0 && ! par.coulombCarriers)
    {
        qFatal("defects.charge != 0 && coulomb.carriers = false");
    }
}

}

#endif // PARAMETERS_H
