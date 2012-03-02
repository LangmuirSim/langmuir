#include "inputparser.h"
#include "sourceagent.h"
#include "simulation.h"
#include "drainagent.h"
#include "cubicgrid.h"
#include "writer.h"
#include "world.h"

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

  // Read command line arguments ( get input file name )
  QStringList args = app.arguments ();

  QString iFileName = "";
  if ( args.size() == 2 ) { iFileName = args.at(1).trimmed(); }
  else
  {
      qDebug() << "correct use is langmuir input.dat";
      qFatal("bad input");
  }

  // Get the current time
  QDateTime begin = QDateTime::currentDateTime();
  QString dateFMT = "MM/dd/yyyy";
  QString timeFMT = "hh:mm:ss:zzz:AP";

  // Create input parser and parse the input file
  InputParser input;
  input.parseKeyValue(iFileName);

  const SimulationParameters& par = input.getParameters(0);

  //Create TextStream for time info
  OutputStream *p_timerStream = 0;
  if (par.outputIsOn)
  {
      p_timerStream = new OutputStream("%path/time.dat",&par,OutputInfo::AppendMode);
      OutputStream& timerStream = *p_timerStream;
      timerStream << right
                  << qSetFieldWidth(20)
                  << qSetRealNumberPrecision(12)
                  << "simulation"
                  << "equilibration"
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
                  << newline
                  << flush;
      timerStream.setRealNumberNotation(QTextStream::SmartNotation);
  }

  for (int i = 0; i < input.steps(); ++i)
  {
      // The time this simulation starts
      QDateTime start = QDateTime::currentDateTime();

      // Get the parameters for this simulation
      SimulationParameters &par = input.getParameters(i);
      par.simulationStart = start;

      // Create the World
      World world(par);

      // Create the simulation
      Simulation sim(world);

      // Save the parameters used for this simulation to a file
      input.saveParameters(i);

      // Perform equilibration steps
      for (int j = 0; j < par.iterationsWarmup; j += par.iterationsPrint)
      {
          // Perform iterations
          sim.performIterations(par.iterationsPrint);

          // Print progress to the screen
          progress( i, j, 0, par.iterationsTotal, start.time().elapsed() );
      }

      // Let the simulation know that equilibration steps are complete
      sim.equilibrated();

      // Perform production steps
      for (int j = 0; j < par.iterationsReal; j += par.iterationsPrint)
      {
          // Perform iterations
          sim.performIterations (par.iterationsPrint);

          // Print progress to the screen
          progress( i, par.iterationsWarmup, j, par.iterationsTotal, start.time().elapsed() );
      }

      // Print progress to the screen
      progress( i, par.iterationsWarmup, par.iterationsReal, par.iterationsTotal, start.time().elapsed() );
      std::cout << "\n";

      // The time this simulation stops
      QDateTime stop = QDateTime::currentDateTime();

      if (par.outputIsOn)
      {
          OutputStream& timerStream = *p_timerStream;
          timerStream << par.currentSimulation
                      << par.iterationsWarmup
                      << par.iterationsReal
                      << world.maxChargeAgents()
                      << start.toString(dateFMT)
                      << start.toString(timeFMT)
                      << stop.toString(dateFMT)
                      << stop.toString(timeFMT)
                      << start.msecsTo(stop) / 1000.0 / 60.0 / 60.0 / 24.0
                      << start.msecsTo(stop) / 1000.0 / 60.0 / 60.0
                      << start.msecsTo(stop) / 1000.0 / 60.0
                      << start.msecsTo(stop) / 1000.0
                      << start.msecsTo(stop)
                      << newline
                      << flush;
      }
  }

  // Get the current time
  QDateTime end  = QDateTime::currentDateTime();
  QDateTime xmas = QDateTime(QDate(end.date().year(),12,25), QTime(0,0));

  if (par.outputIsOn)
  {
      OutputStream& timerStream = *p_timerStream;
      // Report time stats
      timerStream << "begin:" << begin.toString(dateFMT) << begin.toString(timeFMT) << newline;
      timerStream << "end:" << end.toString(dateFMT) << end.toString(timeFMT) << newline;
      timerStream << "days:" << begin.msecsTo(end) / 1000.0 / 60.0 / 60.0 / 24.0 << newline;
      timerStream << "hours:" << begin.msecsTo(end) / 1000.0 / 60.0 / 60.0 << newline;
      timerStream << "mins:" << begin.msecsTo(end) / 1000.0 / 60.0 << newline;
      timerStream << "secs:" << begin.msecsTo(end) / 1000.0 << newline;
      timerStream << "msecs:" << begin.msecsTo(end) << newline;
      timerStream << "xmas:" << QString("%1").arg(end.daysTo(xmas)) << newline << flush;
      delete p_timerStream;
  }
}

void progress( int sim, int warm, int real, int total, double time, bool flush )
{
    double percent = double( warm + real ) / double ( total ) * 100.0;
    double remaining = time / ( warm + real ) * ( total - warm - real );

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
