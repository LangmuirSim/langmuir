#include "inputparser.h"
#include "sourceagent.h"
#include "simulation.h"
#include "drainagent.h"
#include "cubicgrid.h"
#include "world.h"
#include "timer.h"

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
void progress( int sim, int warm, int real, int total, double time, bool flush = true );

int main (int argc, char *argv[])
{
  QApplication app (argc, argv);

  // Read command line arguments
  QStringList args = app.arguments ();

  QString iFileName = "";
  QString oFileName = "";
  if ( args.size() == 2 )
    {
      iFileName = args.at(1);
      QStringList tokens = args.at(1).split("/",QString::SkipEmptyParts);
      oFileName = tokens[tokens.size()-1].split(".",QString::SkipEmptyParts)[0];
    }
  else if ( args.size() == 3 )
    {
      iFileName = args.at(1);
      oFileName = args.at(2);
    }
  else
    {
      qDebug() << "correct use is langmuir input.dat (output.dat)";
      qFatal("bad input");
    }

  InputParser input(iFileName);
  Timer timer;

  for (int i = 0; i < input.steps(); ++i)
  {
      double timeStepStart = timer.now();
      SimulationParameters &par = input.getParameters(i);
      Simulation sim(&par);

      for (int j = 0; j < par.iterationsWarmup; j += par.iterationsPrint)
      {
          sim.performIterations (par.iterationsPrint);
          progress( i, j, 0, par.iterationsTotal, timer.elapsed(timeStepStart) );
      }

      for (int j = 0; j < par.iterationsReal; j += par.iterationsPrint)
      {
          sim.performIterations (par.iterationsPrint);
          progress( i, par.iterationsWarmup, j, par.iterationsTotal, timer.elapsed(timeStepStart) );
      }
      progress( i, par.iterationsWarmup, par.iterationsReal, par.iterationsTotal, timer.elapsed(timeStepStart) );
      std::cout << "\n";
  }

/*
  // Declare output pointers
  // QFile *oFile;                        //summary
  // QFile *iFile;                        //iteration
  // QTextStream *oout;
  // QTextStream *iout;

  // Open and read input file
  InputParser input (iFileName);
  SimulationParameters par;
  input.simulationParameters (&par);

  // Open summary file
  // QDir dir(par.outputPath);
  // if (!dir.exists()) { qFatal("output directory %s does not exist",qPrintable(dir.path()));}

  // oFile = new QFile (dir.absoluteFilePath(oFileName + ".dat"));
  // if (!(oFile->open (QIODevice::WriteOnly | QIODevice::Text)))
  //  {
  //    qDebug () << "Error opening summary file:" << dir.absoluteFilePath(oFileName + ".dat");
  //    app.exit (1);
  //  }
  // oout = new QTextStream (oFile);
  // oout->setRealNumberPrecision (par.outputPrecision);
  // oout->setFieldWidth (par.outputWidth);
  // (*oout) << scientific;

  // Start timer
  Timer timer;

  // Output summary file column titles
  // (*oout) << "working(n)";
  // (*oout) << "temperature(K)";
  // (*oout) << "source(eV)";
  // (*oout) << "drain(eV)";
  // (*oout) << "defects(%)";
  // (*oout) << "trap(%)";
  // (*oout) << "carriers(%)";
  // (*oout) << "reached(%)";
  // (*oout) << "accepted(#/step)";
  // (*oout) << "approxTime(s/step)";
  // (*oout) << "\n";
  // oout->flush ();

  // Total steps per simulation
  int total = par.iterationsWarmup + par.iterationsReal;

  for (int i = 0; i < par.variableSteps; ++i)
    {
      // Get the time we start this simulation
      double timeStepStart = timer.now ();

      // Get simulation parameters for the current step and set up a new object
      input.simulationParameters (&par, i);

      // Set up Simulation
      Simulation *sim = new Simulation(&par,i);

      // Open iteration file for this simulation
      // iFile = new QFile (dir.absoluteFilePath(oFileName + "-i-" + QString::number (i) + ".dat"));
      // if (!(iFile->open (QIODevice::WriteOnly | QIODevice::Text)))
      //   {
      //     qDebug () << "Error opening output file:" << dir.absoluteFilePath(oFileName + "-i-" + QString::number (i) + ".dat");
      //     app.exit (1);
      //   }
      // iout = new QTextStream (iFile);
      // iout->setRealNumberPrecision (par.outputPrecision);
      // iout->setFieldWidth (par.outputWidth);
      // (*iout) << scientific;

      // Output iteration file column titles
      // (*iout) << "move(n)";
      // (*iout) << "temperature(K)";
      // (*iout) << "source(eV)";
      // (*iout) << "drain(eV)";
      // (*iout) << "defects(%)";
      // (*iout) << "trap(%)";
      // (*iout) << "carriers(%)";
      // (*iout) << "reached(%)";
      // (*iout) << "accepted(#/step)";
      // (*iout) << "\n";

      // Perform Warmup
      //unsigned long lastCount = 0;
      for (int j = 0; j < par.iterationsWarmup; j += par.iterationsPrint)
        {
          // Perform Iterations
          sim->performIterations (par.iterationsPrint);

          // Output Iteration Information
          // (*iout) << j
          //  << par.temperatureKelvin
          //  << par.voltageSource
          //  << par.voltageDrain
          //  << par.defectPercentage * 100.00
          //  << par.trapPercentage * 100.0
          //  << par.chargePercentage * 100.0
          //  << double (sim->world()->electrons()->size()) / double (sim->world()->holeSourceL()->maxCarriers()) * 100.0
          //  << double (sim->world()->holeDrainR()->acceptedElectrons() -
          //             lastCount) / double (par.iterationsPrint) << "\n";
          //iout->flush ();
          //lastCount = sim->world()->holeDrainR()->acceptedElectrons();

          progress( i, j, 0, total, timer.elapsed(timeStepStart) );
        }

      // Perform production
      //unsigned long startCount = lastCount;
      for (int j = 0; j < par.iterationsReal; j += par.iterationsPrint)
        {
          // Perform Iterations
          sim->performIterations (par.iterationsPrint);

          // Output Iteration
          //(*iout) << j
          //  << par.temperatureKelvin
          //  << par.voltageSource
          //  << par.voltageDrain
          //  << par.defectPercentage * 100.0
          //  << par.trapPercentage * 100.0
          //  << par.chargePercentage * 100.0
          //  << double (sim->world()->electrons()->size()) / double (sim->world()->holeSourceL()->maxCarriers()) * 100.0
          //  << double (sim->world()->holeDrainR()->acceptedElectrons() -
          //             lastCount) / double (par.iterationsPrint) << "\n";
          //iout->flush ();
          //lastCount = sim->world()->holeDrainR()->acceptedElectrons();

          progress( i, par.iterationsWarmup, j, total, timer.elapsed(timeStepStart) );
        }
        progress( i, par.iterationsWarmup, par.iterationsReal, total, timer.elapsed(timeStepStart) ); std::cout << "\n";

      // Now to output the result of the simulation at this data point
      //(*oout) << i
      //  << par.temperatureKelvin
      //  << par.voltageSource
      //  << par.voltageDrain
      //  << par.defectPercentage * 100.0
      //  << par.trapPercentage * 100.0
      //  << par.chargePercentage * 100.0
      //  << double (sim->world()->electrons()->size()) / double (sim->world()->holeSourceL()->maxCarriers()) * 100.0
      //  << double (lastCount - startCount) / double (par.iterationsReal)
      //  << timer.elapsed (timeStepStart) << "\n";
      //oout->flush ();

      //iFile->close ();
      //delete iFile;
      //delete iout;
      //delete sim;
    }
  //output time
  //(*oout) << reset << "\n" << "approxTime(s): " << timer.elapsed ();

  // Close summary file
  //oFile->close ();
  //delete oFile;
  //delete oout;
  */
}

void progress( int sim, int warm, int real, int total, double time, bool flush )
{
    double percent = double( warm + real ) / double ( total ) * 100.0;
    double remaining = time / ( warm + real ) * ( total - warm - real );

    double factor = 1.0;
    QString unit = "s ";
    if ( time >= 60.0 )
    {
        unit = "m "; factor = 60.0;
        if ( time >= 3600.0 )
        {
            unit = "hr"; factor = 3600.0;
            if ( time >= 86400.0 )
            {
                unit = "d "; factor = 86400.0;
            }
        }
    }
    QString timeString = QString("%1 %2").arg(time/factor, 9,'f',5).arg( unit );

    factor = 1.0;
    unit = "s ";
    if ( remaining >= 60.0 )
    {
        unit = "m "; factor = 60.0;
        if ( remaining >= 3600.0 )
        {
            unit = "hr"; factor = 3600.0;
            if ( remaining >= 86400.0 )
            {
                unit = "d "; factor = 86400.0;
            }
        }
    }
    QString remainingString = QString("%1 %2").arg(remaining/factor, 9,'f',5).arg( unit );

    std::cout << QString( "simulation: %1 | warmup: %2 | real: %3 | percent: %4 % | time: %5| remaining: %6\r")
                 .arg(sim,3)
                 .arg(warm,9)
                 .arg(real,9)
                 .arg(percent,6,'f',2)
                 .arg(timeString)
                 .arg(remainingString)
                 .toStdString();
    if ( flush ) { std::cout.flush(); } else { std::cout << "\n"; }
}
