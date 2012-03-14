#include "sourceagent.h"
#include "simulation.h"
#include "drainagent.h"
#include "cubicgrid.h"
#include "reader.h"
#include "writer.h"
#include "world.h"
#include "openclhelper.h"

#include <QApplication>
#include <QPrinter>
#include <QPainter>
#include <QColor>
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <iostream>

using namespace Langmuir;

// Print information to the screen about simulation status
void progress( int real, int total, double time, bool flush = true );

int main (int argc, char *argv[])
{
    // Get the current time
    QDateTime begin = QDateTime::currentDateTime();
    QString dateFMT = "MM/dd/yyyy";
    QString timeFMT = "hh:mm:ss:zzz:AP";

    // Create the application
    QApplication app(argc, argv);

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

    // Create the simulation
    Simulation sim(world);

    // Save the parameters used for this simulation to a file
    world.reader().save();

    // Print progress to the screen
    progress( 0, par.iterationsReal, begin.time().elapsed() );

    // Perform production steps
    for (int j = 0; j < par.iterationsReal; j += par.iterationsPrint)
    {
        // Perform iterations
        sim.performIterations (par.iterationsPrint);

        // Print progress to the screen
        progress( j + par.iterationsPrint, par.iterationsReal, begin.time().elapsed() );

        // Save a Checkpoint File
        if (par.outputIsOn) world.saveCheckpointFile();
    }

    // Save a Checkpoint File
    if (par.outputIsOn) world.saveCheckpointFile();

    // Print progress to the screen
    std::cout << "\n";

    // The time this simulation stops
    QDateTime stop = QDateTime::currentDateTime();

    // Output Trajectory
    if (par.outputXyz == -1)
    {
        world.logger().reportXYZStream();
    }

    // Output Image of Carriers
    if (par.outputCoulomb == -1)
    {
        world.opencl().launchCoulombKernel1();
        world.logger().saveCoulombEnergy();
    }

    // Output Image of Carriers
    if (par.imageCarriers == -1)
    {
        world.logger().saveCarriersImage();
        world.logger().saveElectronImage();
        world.logger().saveHoleImage();
    }

    // Output time
    if (par.outputIsOn)
    {
        OutputStream timerStream("%path/time.dat",&par);

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

void progress( int real, int total, double time, bool flush )
{
    double percent = double( real ) / double ( total ) * 100.0;
    double remaining = time / ( real ) * ( total - real );

    double factor = 1.0;
    QString unit = "ms";

    if ( time >= 1000.0 )
    {
        unit = "s "; factor = 1000.0;
        if ( time >= 60000.0 )
        {
            unit = "m "; factor = 60000.0;
            if ( time >= 3600000.0 )
            {
                unit = "hr"; factor = 3600000.0;
                if ( time >= 86400000.0 )
                {
                    unit = "d "; factor = 86400000.0;
                }
            }
        }
    }
    QString timeString = QString("%1 %2").arg(time/factor, 9,'f',5).arg( unit );

    factor = 1.0;
    unit = "ms";

    if ( remaining >= 1000.0 )
    {
        unit = "s "; factor = 1000.0;
        if ( remaining >= 60000.0 )
        {
            unit = "m "; factor = 60000.0;
            if ( remaining >= 3600000.0 )
            {
                unit = "hr"; factor = 3600000.0;
                if ( remaining >= 86400000.0 )
                {
                    unit = "d "; factor = 86400000.0;
                }
            }
        }
    }
    QString remainingString = QString("%1 %2").arg(remaining/factor, 9,'f',5).arg( unit );

    std::cout << QString( "step: %1 ps / %2 ps = %3 % | time: %4 | remaining: %5\r")
                 .arg(real,9)
                 .arg(total)
                 .arg(percent,6,'f',2)
                 .arg(timeString)
                 .arg(remainingString)
                 .toStdString();
    if ( flush ) { std::cout.flush(); } else { std::cout << "\n"; }
}
