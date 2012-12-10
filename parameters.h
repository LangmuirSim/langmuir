#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <QDateTime>
#include <QFileInfo>
#include <QDebug>
#include <cmath>
#include <QDir>

namespace Langmuir
{

//! A struct to temporarily store site IDs
struct ConfigurationInfo
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
    QList<qreal> trapPotentials;

    //! a list of flux attempt, success values
    QList<quint64> fluxInfo;
};

/**
  * @brief A struct to store all simulation options
  * To add new variables, follow these steps:
  *   - declare the new variable in the SimulationParameters struct (parameters.h)
  *   - assign the default value of the new variable in the SimulationParameters constructor (parameters.h)
  *   - implement validity checking for the variable in the checkSimulationParameters() function (parameters.h)
  *   - register the variable in the KeyValueParser constructor using the registerVariable() function (keyvalueparser.h)
  *   - to use non-standard types, you must overload certain template functions in variable.h.  See, for example,
  *     overloads for QDateTime in variable.h.
  */
struct SimulationParameters
{
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

    //! output trajectory file (if n < 0, only at the end; if n == 0, never; if n > 0, every n * iterations.print steps)
    qint32 outputXyz;

    //! output electrons in trajectory file (ignored if outputXyz is off)
    bool outputXyzE;

    //! output holes in trajectory file (ignored if outputXyz is off)
    bool outputXyzH;

    //! output defects in trajectory file (ignored if outputXyz is off)
    bool outputXyzD;

    //! output traps in trajectory file (ignored if outputXyz is off)
    bool outputXyzT;

    //! output mode for xyz file (if 0, particle count varies; if 1, particle count is constant using "phantom particles")
    bool outputXyzMode;

    //! output carrier lifetime and pathlength when they are deleted
    bool outputIdsOnDelete;

    //! output coulomb energy for the entire grid (if n < 0, only at the end; if n == 0, never; if n > 0, every n * iterations.print steps)
    qint32 outputCoulomb;

    //! output a checkpoint file every this * iterationsPrint
    qint32 outputStepChk;

    //! output trap potentials in checkpoint files
    bool outputChkTrapPotential;

    //! output grid potential at the start of the simulation, includes the trap potential
    bool outputPotential;

    //! if false, produce no output (useful for LangmuirView)
    bool outputIsOn;

    //! output images of defects
    bool imageDefects;

    //! output images of defects
    bool imageTraps;

    //! output images of carriers (if n < 0, only at the end; if n == 0, never; if n > 0, every n * iterations.print steps)
    qint32 imageCarriers;

    //! if Langmuir, how often to output; if LangmuirView, how many steps between rendering
    qint32 iterationsPrint;

    //! number of simulation steps after equilibration
    qint32 iterationsReal;

    //! number of significant figures used for doubles in output
    qint32 outputPrecision;

    //! width of columns in output, ignored in certain files, like trajectory files
    qint32 outputWidth;

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

    //! the rate at which the left electron source injects charges (overrides default)
    qreal eSourceLRate;

    //! the rate at which the right electron source injects charges (overrides default)
    qreal eSourceRRate;

    //! the rate at which the left hole source injects charges (overrides default)
    qreal hSourceLRate;

    //! the rate at which the right hole source injects charges (overrides default)
    qreal hSourceRRate;

    //! the rate at which the exciton source injects charges (overrides default)
    qreal generationRate;

    //! if true, try to keep the number of charges in the simulation balanced
    bool balanceCharges;

    //! the rate at which all drains accept charges (default, used when eDrainL, etc. are < 0)
    qreal drainRate;

    //! the rate at which the left electron drain accepts charges (overrides default)
    qreal eDrainLRate;

    //! the rate at which the right electron drain accepts charges (overrides default)
    qreal eDrainRRate;

    //! the rate at which the left hole drain accepts charges (overrides default)
    qreal hDrainLRate;

    //! the rate at which the right hole drain accepts charges (overrides default)
    qreal hDrainRRate;

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

    //! the minimum number of charges that must be present to use OpenCL
    qint32 openclThreshold;

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

    //! the number of sites away from a given site used when calculating neighboring sites
    qint32 hoppingRange;

    //! slope of potential along z direction when there are multiple layers (as if there were a gate electrode)
    qreal slopeZ;

    //! if true, use an energy change (source potential and site) and metropolis criterion to calculate injection probability for hole and electron sources (not exciton sources)
    bool sourceMetropolis;

    //! if true, use the Coulomb + Image interaction when calcualting energy change for injection
    bool sourceCoulomb;

    //! the rate at which holes and electrons can combine when they sit upon one another
    qreal recombinationRate;

    //! the number of sites to consider when performing recombinations (0 means same-site, 1 means one-site away and same-site)
    qint32 recombinationRange;

    //! output carrier lifetime and pathlength when holes and electrons encounter one another
    bool outputIdsOnEncounter;

    //! for SimulationParameters::simulationType == "solarcell", multiply SimulationParameters::sourceRate by (Grid::xyPlaneArea)/(SimulationParameters::sourceScaleArea); if <= 0, does not scale rate
    qreal sourceScaleArea;

    //! max threads allowed for QThreadPool - if its <= 0 then the QThread::idealThreadCount is used; note that Qt ignores PBS and SGE so when this isn't set Qt will use all the cores on a node
    qint32 maxThreads;

    SimulationParameters() :

        simulationType         ("transistor"),
        randomSeed             (0),

        gridZ                  (1),
        gridY                  (128),
        gridX                  (128),

        coulombCarriers        (false),
        defectsCharge          (0),

        outputXyz              (0),
        outputXyzE             (true),
        outputXyzH             (true),
        outputXyzD             (true),
        outputXyzT             (true),
        outputXyzMode          (0),

        outputIdsOnDelete      (false),
        outputCoulomb          (0),
        outputStepChk          (1),
        outputChkTrapPotential (true),
        outputPotential        (false),
        outputIsOn             (true),

        imageDefects           (false),
        imageTraps             (false),
        imageCarriers          (0),

        iterationsPrint        (10),
        iterationsReal         (1000),
        outputPrecision        (15),
        outputWidth            (23),
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
        eSourceLRate           (-1.0),
        eSourceRRate           (-1.0),
        hSourceLRate           (-1.0),
        hSourceRRate           (-1.0),
        generationRate         (0.001),
        balanceCharges         (false),

        drainRate              (0.90),
        eDrainLRate            (-1.0),
        eDrainRRate            (-1.0),
        hDrainLRate            (-1.0),
        hDrainRRate            (-1.0),

        useOpenCL              (false),
        workX                  (4),
        workY                  (4),
        workZ                  (4),
        workSize               (256),
        openclThreshold        (256),

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
        simulationStart        (QDateTime::currentDateTime()),
        hoppingRange           (1),
        slopeZ                 (0.00),
        sourceMetropolis       (false),
        sourceCoulomb          (false),
        recombinationRate      (0.00),
        recombinationRange     (0),
        outputIdsOnEncounter   (false),
        sourceScaleArea        (65536),
        maxThreads             (-1)
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
inline void checkSimulationParameters(SimulationParameters& par)
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

    if (par.eSourceLRate >= 0.0)
    {
        if (par.eSourceLRate < 0.0 || par.eSourceLRate > 1.0 )
        {
            qFatal("e.source.l.rate(%f) < 0 || > 1.0", par.eSourceLRate);
        }
    }

    if (par.eSourceRRate >= 0.0)
    {
        if (par.eSourceRRate < 0.0 || par.eSourceRRate > 1.0 )
        {
            qFatal("e.source.r.rate(%f) < 0 || > 1.0", par.eSourceRRate);
        }
    }

    if (par.hSourceLRate >= 0.0)
    {
        if (par.hSourceLRate < 0.0 || par.hSourceLRate > 1.0 )
        {
            qFatal("h.source.l.rate(%f) < 0 || > 1.0", par.hSourceLRate);
        }
    }

    if (par.hSourceRRate >= 0.0)
    {
        if (par.hSourceRRate < 0.0 || par.hSourceRRate > 1.0 )
        {
            qFatal("h.source.r.rate(%f) < 0 || > 1.0", par.hSourceRRate);
        }
    }

    if (par.generationRate >= 0.0)
    {
        if (par.generationRate < 0.0 || par.generationRate > 1.0 )
        {
            qFatal("x.source.rate(%f) < 0 || > 1.0", par.generationRate);
        }
    }

    if (par.drainRate < 0.0 || par.drainRate > 1.0 )
    {
        qFatal("drain.rate(%f) < 0 || > 1.0",par.drainRate);
    }

    if (par.eDrainLRate >= 0.0)
    {
        if (par.eDrainLRate < 0.0 || par.eDrainLRate > 1.0 )
        {
            qFatal("e.drain.l.rate(%f) < 0 || > 1.0", par.eDrainLRate);
        }
    }

    if (par.eDrainRRate >= 0.0)
    {
        if (par.eDrainRRate < 0.0 || par.eDrainRRate > 1.0 )
        {
            qFatal("e.drain.r.rate(%f) < 0 || > 1.0", par.eDrainRRate);
        }
    }

    if (par.hDrainLRate >= 0.0)
    {
        if (par.hDrainLRate < 0.0 || par.hDrainLRate > 1.0 )
        {
            qFatal("h.drain.l.rate(%f) < 0 || > 1.0", par.hDrainLRate);
        }
    }

    if (par.hDrainRRate >= 0.0)
    {
        if (par.hDrainRRate < 0.0 || par.hDrainRRate > 1.0 )
        {
            qFatal("h.drain.r.rate(%f) < 0 || > 1.0", par.hDrainRRate);
        }
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

    if (par.hoppingRange < 0 || par.hoppingRange > 2)
    {
        qFatal("hopping.range(%d) < 0 || > 2",par.hoppingRange);
    }

    if (!par.sourceMetropolis)
    {
        if (par.sourceCoulomb)
        {
            qFatal("source.metropolis = false, yet source.coulomb = true");
        }
    }

    if (par.recombinationRate < 0.0 || par.recombinationRate > 1.0 )
    {
        qFatal("recombination.rate(%f) < 0 || > 1.0",par.recombinationRate);
    }

    if (par.recombinationRate > 0 && par.simulationType != "solarcell")
    {
        qFatal("recombination.rate(%f) > 0, yet simulation.type != solarcell",par.recombinationRate);
    }

    if (par.recombinationRate && !((par.electronPercentage > 0) && (par.holePercentage > 0)))
    {
        qFatal("recombination.rate(%f) > 0, yet electron.percentage = %.5f and hole.percentage = %.5f",
               par.recombinationRate, par.electronPercentage, par.holePercentage);
    }

    if (par.outputIdsOnEncounter && (par.simulationType != "solarcell"))
    {
        qFatal("output.ids.on.encounter == true, yet simulation.type != solarcell");
    }

    if (par.outputIdsOnEncounter && !((par.electronPercentage > 0) && (par.holePercentage > 0)))
    {
        qFatal("output.ids.on.encounter == true, yet electron.percentage = %.5f and hole.percentage = %.5f",
               par.electronPercentage, par.holePercentage);
    }

    if (((par.outputXyzE == false) && (par.outputXyzH == false) && (par.outputXyzD == false) && (par.outputXyzT == false)) && par.outputXyz > 0)
    {
        qFatal("output.xyz > 0, yet output.xyz.e && output.xyz.h && output.xyz.d && output.xyz.t are all false");

    }

    if (par.outputXyzMode < 0 || par.outputXyzMode > 1)
    {
        qFatal("output.xyz.mode must be 0 or 1");
    }

    if (par.openclThreshold <= 0)
    {
        qFatal("opencl.threshold must be >= 0");
    }

    if (par.balanceCharges && par.simulationType != "solarcell")
    {
        qFatal("balance.charges == true, yet simulation.type != solarcell");
    }
}

}

#endif // PARAMETERS_H
