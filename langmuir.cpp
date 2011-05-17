#include "inputparser.h"
#include "cubicgrid.h"
#include "world.h"
#include "simulation.h"
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

using namespace std;
using namespace Langmuir;

int main (int argc, char *argv[])
{
  QApplication app (argc, argv);

  // read command line arguments
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

  // Declare output pointers
  QFile *oFile;                        //summary
  QFile *iFile;                        //iteration
  QFile *tFile;                        //trajectory
  QTextStream *oout;
  QTextStream *iout;
  QTextStream *tout;

  // Open and read input file
  InputParser input (iFileName);
  SimulationParameters par;
  input.simulationParameters (&par);

  // Open summary file 
  oFile = new QFile (oFileName + ".dat");
  if (!(oFile->open (QIODevice::WriteOnly | QIODevice::Text)))
    {
      qDebug () << "Error opening summary file:" << oFileName + ".dat";
      app.exit (1);
    }
  oout = new QTextStream (oFile);
  oout->setRealNumberPrecision (par.outputPrecision);
  oout->setFieldWidth (par.outputWidth);
  (*oout) << scientific;

  //start timer
  Timer timer;

  // Output summary file column titles
  (*oout) << "working(n)";
  (*oout) << "temperature(K)";
  (*oout) << "source(eV)";
  (*oout) << "drain(eV)";
  (*oout) << "defects(%)";
  (*oout) << "trap(%)";
  (*oout) << "carriers(%)";
  (*oout) << "reached(%)";
  (*oout) << "accepted(#/step)";
  (*oout) << "approxTime(s/step)";
  (*oout) << "\n";
  oout->flush ();

  for (int i = 0; i < par.variableSteps; ++i)
    {

      double timeStepStart = timer.now ();

      qDebug() << "Simulation: " << i;

      // Get simulation parameters for the current step and set up a new object
      input.simulationParameters (&par, i);

      // Set up a simulation
      Simulation sim (&par);

      // Open iteration file for this simulation
      iFile = new QFile (oFileName + "-i-" + QString::number (i) + ".dat");
      if (!(iFile->open (QIODevice::WriteOnly | QIODevice::Text)))
        {
          qDebug () << "Error opening output file:" << oFileName + "-i-" +
            QString::number (i) + ".dat";
          app.exit (1);
        }
      iout = new QTextStream (iFile);
      iout->setRealNumberPrecision (par.outputPrecision);
      iout->setFieldWidth (par.outputWidth);
      (*iout) << scientific;

      // Open trajectoryFile for this simulation
      if (par.outputXyz)
        {
          tFile = new QFile (oFileName + QString::number (i) + ".xyz");
          if (!(tFile->open (QIODevice::WriteOnly | QIODevice::Text)))
            {
              qDebug () << "Error opening trajectory file:" << oFileName +
                "-i-" + QString::number (i) + ".xyz";
              app.exit (1);
            }
          tout = new QTextStream (tFile);
        }

      // Output iteration file column titles
      (*iout) << "move(n)";
      (*iout) << "temperature(K)";
      (*iout) << "source(eV)";
      (*iout) << "drain(eV)";
      (*iout) << "defects(%)";
      (*iout) << "trap(%)";
      (*iout) << "carriers(%)";
      (*iout) << "reached(%)";
      (*iout) << "accepted(#/step)";
      (*iout) << "\n";

      // Perform Warmup
      unsigned long lastCount = 0;
      for (int j = 0; j < par.iterationsWarmup; j += par.iterationsPrint)
        {

          // Perform Iterations
          sim.performIterations (par.iterationsPrint);

          // Output Trajectory
          if (par.outputXyz)
            sim.getGrid ()->print3D ((*tout));

          // Output Iteration Information
          qDebug() << "    warmup: " << j;
          (*iout) << j
            << par.temperatureKelvin
            << par.voltageSource
            << par.voltageDrain
            << par.defectPercentage * 100.00
            << par.trapPercentage * 100.0
            << par.chargePercentage * 100.0
            << double (sim.charges ()) / double (sim.getMaxCharges ()) * 100.0
            << double (sim.totalChargesAccepted () -
                       lastCount) / double (par.iterationsPrint) << "\n";
          iout->flush ();
          lastCount = sim.totalChargesAccepted ();

          //sim.printGrid();
        }

      // Perform production
      unsigned long startCount = lastCount;
      for (int j = 0; j < par.iterationsReal; j += par.iterationsPrint)
        {

          // Perform Iterations
          sim.performIterations (par.iterationsPrint);

          // Output Trajectory
          if (par.outputXyz)
            sim.getGrid ()->print3D ((*tout));

          // Output Iteration
          qDebug() << "      step: " << j;
          (*iout) << j
            << par.temperatureKelvin
            << par.voltageSource
            << par.voltageDrain
            << par.defectPercentage * 100.0
            << par.trapPercentage * 100.0
            << par.chargePercentage * 100.0
            << double (sim.charges ()) / double (sim.getMaxCharges ()) * 100.0
            << double (sim.totalChargesAccepted () -
                       lastCount) / double (par.iterationsPrint) << "\n";
          iout->flush ();
          lastCount = sim.totalChargesAccepted ();

          //sim.printGrid();
        }

      // Now to output the result of the simulation at this data point
      (*oout) << i
        << par.temperatureKelvin
        << par.voltageSource
        << par.voltageDrain
        << par.defectPercentage * 100.0
        << par.trapPercentage * 100.0
        << par.chargePercentage * 100.0
        << double (sim.charges ()) / double (sim.getMaxCharges ()) * 100.0
        << double (lastCount - startCount) / double (par.iterationsReal)
        << timer.elapsed (timeStepStart) << "\n";
      oout->flush ();

      iFile->close ();
      delete iFile;
      delete iout;

      if (par.outputXyz)
        {
          tFile->close ();
          delete tFile;
          delete tout;
        }

      qDebug() << "";
    }
  //output time
  (*oout) << reset << "\n" << "approxTime(s): " << timer.elapsed ();

  // Close summary file
  oFile->close ();
  delete oFile;
  delete oout;

  qDebug() << "Simulation complete - destroy our objects...";

}
