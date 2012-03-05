#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <QDateTime>
#include <QFileInfo>
#include <QDebug>
#include <cmath>
#include <QDir>

namespace Langmuir
{

struct Parameters
{
};

struct SimulationParameters
{
    //! tells Langmuir how to set up the Sources and Drains: (\b\c "transistor", \b\c "solarcell")
    /*!
      - \b\c "transistor" creates an ElectronSource on the left and an electronDrain on the right
      - \b\c "solarcell" creates 2 ElectronDrain and 2 holeDrain, one on each side of the grid, and an ExcitonSource
      */
    QString simulationType;

    //! seed the random number generator, if negative, uses the current time (making seperate runs random)
    unsigned int randomSeed;

    //! the number of sites per layer, at least one
    int gridZ;

    //! the number of sites along the device width, at least one
    int gridY;

    //! the number of sites along the device length, at least one
    int gridX;

    //! turn on Coulomb interactions between ChargeAgents
    /*!
      \see SimulationParameters::useOpenCL
      \see SimulationParameters::workSize
      */
    bool coulombCarriers;

    //! turn on Coulomb interactions between ChargeAgents and Defects
    /*!
      \see SimulationParameters::useOpenCL
      \see SimulationParameters::defectsCharge
      */
    bool coulombDefects;

    //! the charge of defect sites
    /*!
      \see SimulationParameters::useOpenCL
      \see SimulationParameters::coulombDefects
      */
    int defectsCharge;

    //! output trajectory file every interationsPrint
    bool outputXyz;

    //! output carrier lifetime and pathlength when they are deleted
    bool outputIdsOnDelete;

    //! output coulomb energy for the entire grid every iterationsPrint
    /*!
      \warning computationally expensive, will slow down the code
      \warning files are large, will eat up a lot of hard drive space
      \warning requires OpenCL
      \see SimulationParameters::useOpenCL
      \see SimulationParameters::workX
      \see SimulationParameters::workY
      \see SimulationParameters::workZ
      */
    bool outputCoulomb;

    //! output grid potential at the start of the simulation, includes the trap potential
    bool outputPotential;

    //! output images of defects and traps at the start of the simulation
    bool outputImage;

    //! if false, produce no output (useful for LangmuirView)
    bool outputIsOn;

    //! if true, overwrite files; if false, backup files
    bool outputOverwrite;

    //! if Langmuir, how often to output; if LangmuirView, how many steps between rendering
    int iterationsPrint;

    //! number of simulation steps after equilibration
    /*!
      \see SimulationParameters::iterationsWarmup
      */
    int iterationsReal;

    //! number of equilibration steps
    /*!
      \see SimulationParameters::iterationsReal
      */
    int iterationsWarmup;

    //! number of significant figures used for doubles in output
    /*!
      \see SimulationParameters::outputWidth
      */
    int outputPrecision;

    //! width of columns in output, ignored in certain files, like trajectory files
    /*!
      \see SimulationParameters::outputPrecision
      */
    int outputWidth;

     //! the output directory, defaults to the current working directory
    QString outputPath;

    //! the percent of the grid that is reserved for electrons, between 0 and 1
    double electronPercentage;

    //! the percent of the grid that is reserved for holes, between 0 and 1
    double holePercentage;

    //! if true, place charges randomly before the simulation starts
    bool seedCharges;

    //! the percent of the grid that is reserved for electrons, between 0 and 1
    double defectPercentage;

    //! the percent of the grid that is reserved for traps, between 0 and 1
    double trapPercentage;

    //! the potential of traps
    double trapPotential;

    //! the standard deviation of trap sites
    double gaussianStdev;

    //! the percent of the traps to be placed and grown upon to form islands
    double seedPercentage;

    //! the potential on the right side of the grid, used in setting up an electric field
    double voltageRight;

    //! the potential on the left side of the grid, used in setting up an electric field
    double voltageLeft;

    //! the temperature used in the boltzmann factor
    double temperatureKelvin;

    //! the rate at which all sources inject charges
    double sourceRate;

    //! the rate at which all drains accept chargew
    double drainRate;

    //! if true, try to use OpenCL to speed up Coulomb interaction calculations
    bool useOpenCL;

    //! the x size of OpenCL 3DRange kernel work groups - only needed if using SimulationParameters::outputCoulomb
    /*!
      make it a multiple of the SimulationParameters::gridX
      \see SimulationParameters::outputCoulomb
      \see SimulationParameters::useOpenCL
      \see SimulationParameters::workY
      \see SimulationParameters::workZ
      \see SimulationParameters::gridX
      */
    int workX;

    //! the y size of OpenCL 3DRange kernel work groups - only needed if using SimulationParameters::outputCoulomb
    /*!
      make it a multiple of the SimulationParameters::gridY
      \see SimulationParameters::outputCoulomb
      \see SimulationParameters::useOpenCL
      \see SimulationParameters::workX
      \see SimulationParameters::workZ
      \see SimulationParameters::gridY
      */
    int workY;

    //! the z size of OpenCL 3DRange kernel work groups - only needed if using SimulationParameters::outputCoulomb
    /*!
      make it a multiple of the SimulationParameters::gridZ
      \see SimulationParameters::outputCoulomb
      \see SimulationParameters::useOpenCL
      \see SimulationParameters::workX
      \see SimulationParameters::workY
      \see SimulationParameters::gridZ
      */
    int workZ;

    //! the size of OpenCL 1DRange kernel work groups
    /*!
      make it a power of 2
      \see SimulationParameters::outputCoulomb
      \see SimulationParameters::useOpenCL
      \warning the speed of Coulomb calculations is very sensitive to this number,
               try various powers of 2 for a few steps to find an optimal value
      */
    int workSize;

    //! physical constant, the boltzmann constant
    double boltzmannConstant;

    //! physical constant, the dielectic constant
    double dielectricConstant;

    //! physical constant, the elementary charge
    double elementaryCharge;

    //! physical constant, the permittivity of free spece
    double permittivitySpace;

    //! size constant, the size associated with grid sites (~1nm)
    double gridFactor;

    //! the cut off for Coulomb interations
    int electrostaticCutoff;

    //! a compilation of physical constants
    double electrostaticPrefactor;

    //! a compilation of physical constants
    double inverseKT;

    //! if true, OpenCL can be used on this platform
    bool okCL;

    //! the current step of the simulation
    unsigned int currentStep;

    //! the current simulation
    unsigned int currentSimulation;

    //! the stub to use when naming output files
    QString outputStub;

    //! the total number of simulation steps
    int iterationsTotal;

    //! an xyz file containing a preconfigured grid
    QString configurationFile;

    //! a file containing a preconfigured potential
    QString potentialFile;

    //! the time this simulation started
    QDateTime simulationStart;

    SimulationParameters() :

        simulationType         ("transistor"),
        randomSeed             (0),

        gridZ                  (1),
        gridY                  (128),
        gridX                  (128),

        coulombCarriers        (false),
        coulombDefects         (false),
        defectsCharge          (-1),

        outputXyz              (false),
        outputIdsOnDelete      (false),
        outputCoulomb          (false),
        outputPotential        (false),
        outputImage            (false),
        outputIsOn             (true),
        outputOverwrite        (false),

        iterationsPrint        (10),
        iterationsReal         (1000),
        iterationsWarmup       (1000),
        outputPrecision        (12),
        outputWidth            (20),
        outputPath             (QDir::currentPath()),

        electronPercentage     (0.01),
        holePercentage         (0.00),
        seedCharges            (false),
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
        currentSimulation      (0),
        outputStub             ("out"),
        iterationsTotal        (0),

        configurationFile      (""),

        simulationStart (QDateTime::fromMSecsSinceEpoch(0))
    {
        setCalculatedValues();
        check();
    }

    //! sets parameters that depend upon other parameters
    void setCalculatedValues()
    {
        electrostaticPrefactor  = elementaryCharge /(4.0 * M_PI * dielectricConstant * permittivitySpace * gridFactor);
        inverseKT = elementaryCharge /(boltzmannConstant * temperatureKelvin);
        iterationsTotal = iterationsReal + iterationsWarmup;
        if (outputWidth < 8 ) { outputWidth = 8; }
        if (outputWidth < (outputPrecision + 8)) { outputWidth = outputPrecision + 8; }
    }

    //! check the parameters, making sure they are valid
    void check()
    {
        setCalculatedValues();
        // simulation type
        if (!(QStringList()<<"transistor"<<"solarcell").contains(simulationType))
        {
            qFatal("simulation.type(%s) must be transistor or solarcell",qPrintable(simulationType));
        }

        // grid size
        if (gridZ < 1)
        {
            qFatal("grid.x(%d) >= 1",gridX);
        }
        if (gridY < 1)
        {
            qFatal("grid.y(%d) >= 1",gridY);
        }
        if (gridX < 1)
        {
            qFatal("grid.z(%d) >= 1",gridZ);
        }

        // output
        if (iterationsPrint <= 0 )
        {
            qFatal("iterations.print(%d) <= 0",iterationsPrint);
        }
        if (iterationsReal < 0 )
        {
            qFatal("iterations.real(%d) < 0",iterationsReal);
        }
        if (iterationsWarmup < 0 )
        {
            qFatal("iterations.warmup(%d) < 0",iterationsWarmup);
        }
        if ( iterationsReal % iterationsPrint != 0 )
        {
            qFatal("iterations.real(%d) %% iterations.print(%d) != 0",iterationsReal,iterationsPrint);
        }
        if ( iterationsWarmup % iterationsPrint != 0 )
        {
            qFatal("iterations.warmup(%d) %% iterations.print(%d) != 0",iterationsWarmup,iterationsPrint);
        }
        if ( iterationsTotal < 0 )
        {
            qFatal("iterations.total(%d) < 0",iterationsTotal);
        }

        // percentages
        if ( electronPercentage < 0.0 || electronPercentage > 1.0 )
        {
            qFatal("electron.pecentage(%f) < 0 || > 1.0",electronPercentage);
        }
        if ( holePercentage     < 0.0 || holePercentage     > 1.0 )
        {
            qFatal("hole.pecentage(%f) < 0 || > 1.0",holePercentage);
        }
        if ( defectPercentage   < 0.0 || defectPercentage   > 1.0 )
        {
            qFatal("defect.pecentage(%f) < 0 || > 1.0",defectPercentage);
        }
        if ( trapPercentage     < 0.0 || trapPercentage     > 1.0 )
        {
            qFatal("trap.pecentage(%f) < 0 || > 1.0",trapPercentage);
        }
        if ( seedPercentage     < 0.0 || seedPercentage     > 1.0 )
        {
            qFatal("seed.pecentage(%f) < 0 || > 1.0",seedPercentage);
        }
        if (defectPercentage > 1.00 - trapPercentage)
        {
            qFatal("trap.percentage(%f) > 1.0 - trap.percentage(%f)",defectPercentage,trapPercentage);
        }
        if ( sourceRate < 0.0 || sourceRate > 1.0 )
        {
            qFatal("source.rate(%f) < 0 || > 1.0",sourceRate);
        }
        if ( drainRate < 0.0 || drainRate > 1.0 )
        {
            qFatal("drain.rate(%f) < 0 || > 1.0",drainRate);
        }

        // other
        if ( gaussianStdev < 0.00 )
        {
            qFatal("gaussian.stdev < 0.0");
        }
        if ( temperatureKelvin < 0.0 )
        {
            qFatal("temperature.kelvin < 0.0");
        }

        if ( ! configurationFile.isEmpty() )
        {
            QFileInfo info(configurationFile);
            if (!info.exists())
            {
                info = QFileInfo(QDir(outputPath),configurationFile);
            }
            if (!info.exists() || !info.isFile())
            {
                qFatal("configuration.file does not exist"
                       "\n\t%s",qPrintable(info.absoluteFilePath()));
            }
        }
    }
};

}

#endif // PARAMETERS_H
