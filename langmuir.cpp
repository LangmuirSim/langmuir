#include "sourceagent.h"
#include "simulation.h"
#include "drainagent.h"
#include "cubicgrid.h"
#include "keyvalueparser.h"
#include "writer.h"
#include "world.h"
#include "openclhelper.h"
#include "checkpointer.h"
#include "parameters.h"

#include <QApplication>
#include <QPrinter>
#include <QPainter>
#include <QColor>
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QThreadPool>

using namespace Langmuir;

QTextStream& progress(QTextStream &stream, SimulationParameters& par)
{
    qint64 msec  = par.simulationStart.msecsTo(QDateTime::currentDateTime());
    double sec   = msec / 1000.0;
    qint64 left  = par.iterationsReal - par.currentStep;
    QString info = QString("step: %1 remaining: %2 elapsed: %3 s")
            .arg(par.currentStep, 35)
            .arg(left, 35)
            .arg(sec, 35, 'f', 5);
    return stream << qPrintable(info);
}

void alterMaxThreads(SimulationParameters &par)
{
    if (par.maxThreads <= 0)
    {
        return;
    }

    QThreadPool& threadPool = *QThreadPool::globalInstance();
    int maxThreadCount = threadPool.maxThreadCount();

    if (par.maxThreads == maxThreadCount)
    {
        return;
    }

    if (par.maxThreads > maxThreadCount)
    {
        qWarning("warning: requesting more threads than recommended\n"
                 "QThread::idealThreadCount = %d\n"
                 "max.threads = %d", maxThreadCount, par.maxThreads);
    }

    threadPool.setMaxThreadCount(par.maxThreads);
}

int main (int argc, char *argv[])
{
    // Create stdout
    QTextStream qout(stdout);

    // Get the current time
    QDateTime begin = QDateTime::currentDateTime();
    QString dateFMT = "MM/dd/yyyy";
    QString timeFMT = "hh:mm:ss:zzz:AP";

    // Create the application
    QCoreApplication app(argc, argv);

    // Get the input file
    QStringList args = app.arguments();
    if (args.size() != 2)
    {
        qFatal("correct use is: langmuir input.dat");
    }
    QString inputFile = args.at(1);

    // Create the world
    World world(inputFile);

    // Get the simulation Parameters
    SimulationParameters &par = world.parameters();

    // Set the max number of QThreads in QThreadPool
    alterMaxThreads(par);

    // Save the parameters
    world.keyValueParser().save("%stub.parm");

    // Create the simulation
    Simulation sim(world);

    // Perform production steps
    for (int j = par.currentStep; j < par.iterationsReal; j += par.iterationsPrint)
    {
        // Perform iterations
        sim.performIterations (par.iterationsPrint);

        // Output to the screen
        progress(qout, par) << '\r'; qout.flush();

        // Save a Checkpoint File
        if (par.outputIsOn) world.checkPointer().save();
    }
    progress(qout, par) << '\n'; qout.flush();

    // Save a Checkpoint File
    if (par.outputIsOn) world.checkPointer().save();

    // Output Trajectory
    if (par.outputXyz == -1)
    {
        world.logger().reportXYZStream();
    }

    // Output Image of Carriers
    if (par.outputCoulomb == -1)
    {
        if (world.numChargeAgents() > 0)
        {
            world.opencl().launchCoulombKernel1();
            world.logger().saveCoulombEnergy();
        }
    }

    // Output Image of Carriers
    if (par.imageCarriers == -1)
    {
        world.logger().saveCarriersImage();
        world.logger().saveElectronImage();
        world.logger().saveHoleImage();
    }

    // The time this simulation stops
    QDateTime stop = QDateTime::currentDateTime();

    // Output time
    if (par.outputIsOn)
    {
        OutputStream timerStream("%stub.time",&par);

        timerStream << right
                    << qSetFieldWidth(20)
                    << qSetRealNumberPrecision(12)
                    << "real"
                    << "carriers"
                    << "date_i"
                    << "time_i"
                    << "date_f"
                    << "time_f"
                    << "days"
                    << "hours"
                    << "min"
                    << "secs"
                    << "msecs"
                    << newline;
        timerStream.setRealNumberNotation(QTextStream::SmartNotation);
        timerStream << par.iterationsReal
                    << world.maxChargeAgents()
                    << begin.toString(dateFMT)
                    << begin.toString(timeFMT)
                    << stop.toString(dateFMT)
                    << stop.toString(timeFMT)
                    << begin.msecsTo(stop) / 1000.0 / 60.0 / 60.0 / 24.0
                    << begin.msecsTo(stop) / 1000.0 / 60.0 / 60.0
                    << begin.msecsTo(stop) / 1000.0 / 60.0
                    << begin.msecsTo(stop) / 1000.0
                    << begin.msecsTo(stop)
                    << newline
                    << flush;
    }
}
