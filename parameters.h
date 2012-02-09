#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <QDir>
#include <cmath>

namespace Langmuir
{

struct SimulationParameters
{
    QString simulationType;

    int randomSeed;

    int gridZ;
    int gridY;
    int gridX;

    bool coulombCarriers;
    bool coulombDefects;
    int defectsCharge;

    bool outputIdsOnIteration;
    bool outputIdsAtStart;
    bool outputIdsOnDelete;

    bool outputCoulomb;
    bool outputPotential;
    bool outputImage;
    bool outputIsOn;
    bool outputOverwrite;

    int iterationsPrint;
    int iterationsReal;
    int iterationsWarmup;
    int outputPrecision;
    int outputWidth;
    QString outputPath;

    double electronPercentage;
    double holePercentage;
    bool seedCharges;
    double defectPercentage;
    double trapPercentage;
    double trapPotential;
    double gaussianAverg;
    double gaussianStdev;
    double seedPercentage;

    double voltageDrain;
    double voltageSource;
    double temperatureKelvin;

    double sourceRate;
    double drainRate;

    bool useOpenCL;
    int workX;
    int workY;
    int workZ;
    int workSize;
    QString kernelsPath;

    double boltzmannConstant;
    double dielectricConstant;
    double elementaryCharge;
    double permittivitySpace;
    double gridFactor;
    int electrostaticCutoff;
    double electrostaticPrefactor;
    double inverseKT;
    bool okCL;
    unsigned int currentStep;
    unsigned int currentSimulation;
    QString outputStub;
    int iterationsTotal;
    int simulationSteps;

    SimulationParameters() :

        simulationType         ("transistor"),
        randomSeed             (-1),

        gridZ                  (1),
        gridY                  (128),
        gridX                  (128),

        coulombCarriers        (false),
        coulombDefects         (false),
        defectsCharge          (-1),

        outputIdsOnIteration   (false),
        outputIdsAtStart       (false),
        outputIdsOnDelete      (false),

        outputCoulomb          (false),
        outputPotential        (false),
        outputImage            (false),
        outputIsOn             (true),
        outputOverwrite        (false),

        iterationsPrint        (10),
        iterationsReal         (1000),
        iterationsWarmup       (1000),
        outputPrecision        (13),
        outputWidth            (20),
        outputPath             (QDir::currentPath()),

        electronPercentage     (0.01),
        holePercentage         (0.00),
        seedCharges            (false),
        defectPercentage       (0.00),
        trapPercentage         (0.00),
        trapPotential          (0.10),
        gaussianAverg          (0.00),
        gaussianStdev          (0.00),
        seedPercentage         (1.0),

        voltageDrain           (0.00),
        voltageSource          (0.00),
        temperatureKelvin      (300.0),

        sourceRate             (0.90),
        drainRate              (0.90),

        useOpenCL              (false),
        workX                  (4),
        workY                  (4),
        workZ                  (4),
        workSize               (256),
        kernelsPath            (QDir::currentPath()),

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
        simulationSteps        (1)
    {
        setCalculatedValues();
        check();
    }

    void setCalculatedValues()
    {
        electrostaticPrefactor  = elementaryCharge /(4.0 * M_PI * dielectricConstant * permittivitySpace * gridFactor);
        inverseKT = elementaryCharge /(boltzmannConstant * temperatureKelvin);
        iterationsTotal = iterationsReal + iterationsWarmup;
        if (outputWidth < 8 ) { outputWidth = 8; }
        if (outputWidth < (outputPrecision + 8)) { outputWidth = outputPrecision + 8; }
    }

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
        if ( simulationSteps <= 0 )
        {
            qFatal("simulation.steps(%d) <= 0",simulationSteps);
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

    }
};

}

#endif // PARAMETERS_H
