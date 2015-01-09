#include "simulation.h"
#include "keyvalueparser.h"
#include "writer.h"
#include "world.h"
#include "checkpointer.h"
#include "nodefileparser.h"
#include "parameters.h"
#include "clparser.h"

#include <QApplication>

using namespace LangmuirCore;

int main (int argc, char *argv[])
{
    // Get the current time
    QDateTime begin = QDateTime::currentDateTime();
    QString dateFMT = "MM/dd/yyyy";
    QString timeFMT = "hh:mm:ss:zzz:AP";

    qDebug("langmuir: %s", qPrintable(begin.toString(dateFMT)));
    qDebug("langmuir: %s", qPrintable(begin.toString(timeFMT)));

    // Create the application
    QCoreApplication app(argc, argv);

    // Get the command line arguments
    QStringList args = app.arguments();

    // Parse the arguments
    CommandLineParser clparser;
    clparser.add("-n", "cores", "the number of cores to use");
    clparser.add("--gpu", "gpu", "index of gpu to use");
    clparser.addPositional("input", "input file");
    clparser.parse(args);

    // Figure out cores
    int cores = clparser.get<int>("cores", -1);
    int gpuID = clparser.get<int>("gpu", -1);

    // Get the input file
    QString inputFile = clparser.get<QString>("input", "sim.inp");

    // Create the world
    World world(inputFile, cores, gpuID);
    world.logger().initialize();

    // Get the simulation Parameters
    SimulationParameters &par = world.parameters();

    // Save the parameters
    world.keyValueParser().save("%stub.parm");

    // Create the simulation
    Simulation sim(world);

    qDebug("langmuir: performing iterations...");

    // Perform production steps
    for (int j = par.currentStep; j < par.iterationsReal; j += par.iterationsPrint)
    {
        // Perform iterations
        sim.performIterations (par.iterationsPrint);
    }

    // The time this simulation stops
    QDateTime stop = QDateTime::currentDateTime();

    qDebug("langmuir: %s", qPrintable(stop.toString(dateFMT)));
    qDebug("langmuir: %s", qPrintable(stop.toString(timeFMT)));

    // Output some stuff
    if (par.outputIsOn)
    {
        // Save a Checkpoint File
        if (par.outputIsOn) world.checkPointer().save();

        // Output time
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

    qDebug("langmuir: exited successfully");
}
