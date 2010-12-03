#include "inputparser.h"
#include "cubicgrid.h"
#include "simulation.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>

using namespace std;
using namespace Langmuir;

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
  QStringList args = app.arguments();

  if (args.size() < 2) {
    qDebug() << "correct use is langmuir input.dat (output.dat)";
    throw(std::invalid_argument("bad input"));
  }

  QString  inputFileName = args.at(1);
  QString outputFileName = args.size() > 2 ? args.at(2) : args.at(1).split(".")[0];

  // Open input file
  QFile inputFile(inputFileName);
  if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Error opening intput file:" << inputFileName;
    app.exit(1);
  }
  inputFile.close();
  InputParser input(inputFileName);
  SimulationParameters par;
  input.simulationParameters(&par);

  // Open summary file 
  QFile outputFile(outputFileName+".dat");
  if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qDebug() << "Error opening summary file:" << outputFileName + ".dat";
    app.exit(1);
  }
  QTextStream out(&outputFile);
  out.setRealNumberPrecision(5);
  out.setFieldWidth(20);
  out << scientific;

  // Open log file 
  QFile logFile(outputFileName+".log");
  if (!logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qDebug() << "Error opening log file:" << outputFileName + ".log";
    app.exit(1);
  }
  QTextStream log(&logFile);

  // Find start time
  QDateTime startDateTime(QDateTime::currentDateTime());

  // Output log title
  log << "Langmuir Simulation Code"       << "\n";
  log << "Hutchison Group"                << "\n";
  log << "\n\n";

  // Output summary file column titles
  out << "working(n)";
  out << "temperature(K)";
  out << "source(eV)";
  out << "drain(eV)";
  out << "defects(%)";
  out << "trap(%)";
  out << "carriers(%)";
  out << "reached(%)";
  out << "accepted(#/step)";
  out << "\n";
  out.flush();

  for (int i = 0; i < input.steps(); ++i) {

    qDebug() << "Simulation: " << i;

    // Get simulation parameters for the current step and set up a new object
    input.simulationParameters(&par, i);

    // Set up a simulation
    Simulation sim(&par);

    // Write log file for this step
    log << input << "\n" << par << "\n\n\n\n";
    log.flush();

    // Open iteration file for this simulation
    QFile iterFile(outputFileName+QString::number(i)+".dat");
    if (!iterFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
      qDebug() << "Error opening output file:" << outputFileName+"-i-"+QString::number(i)+".dat";
      app.exit(1);
    }
    QTextStream iterOut(&iterFile);
    iterOut.setRealNumberPrecision(5);
    iterOut.setFieldWidth(20);
    iterOut << scientific;

    // Open trajectoryFile for this simulation
    QFile trajFile(outputFileName+QString::number(i)+".xyz");
    if (!trajFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
      qDebug() << "Error opening trajectory file:" << outputFileName+"-i-"+QString::number(i)+".xyz";
      app.exit(1);
    }
    QTextStream trajOut(&trajFile);

    // Output iteration file column titles
    iterOut << "move(n)";
    iterOut << "temperature(K)";
    iterOut << "source(eV)";
    iterOut << "drain(eV)";
    iterOut << "defects(%)";
    iterOut << "trap(%)";
    iterOut << "carriers(%)";
    iterOut << "reached(%)";
    iterOut << "accepted(#/step)";
    iterOut << "\n";
 
    // Perform Warmup
    unsigned long lastCount = 0;
    for (int j = 0; j < par.iterationsWarmup; j += par.iterationsPrint) {

     // Perform Iterations
     sim.performIterations(par.iterationsPrint);

     // Output Trajectory
     if ( par.iterationsXYZ ) sim.getGrid()->print3D(trajOut);

     // Output Log
     iterOut  << j
              << par.temperatureKelvin
              << par.voltageSource
              << par.voltageDrain
              << par.defectPercentage * 100.00
              << par.trapPercentage * 100.0
              << par.chargePercentage * 100.0
              << double(sim.charges()) / double(sim.getMaxCharges()) * 100.0
              << double(sim.totalChargesAccepted()-lastCount) / par.iterationsPrint << "\n";
      iterOut.flush();
      lastCount = sim.totalChargesAccepted();
    }

    // Perform production
    unsigned long startCount = lastCount;
    for (int j = 0; j < par.iterationsReal; j += par.iterationsPrint) {

      // Perform Iterations
      sim.performIterations(par.iterationsPrint);

      // Output Trajectory
      if ( par.iterationsXYZ ) sim.getGrid()->print3D(trajOut);

      // Output Iteration
      qDebug() << "      step: " << j;
      iterOut  << j
               << par.temperatureKelvin
               << par.voltageSource
               << par.voltageDrain
               << par.defectPercentage * 100.0
               << par.trapPercentage * 100.0
               << par.chargePercentage * 100.0
               << double(sim.charges()) / double(sim.getMaxCharges()) * 100.0
               << double(sim.totalChargesAccepted()-lastCount) / par.iterationsPrint << "\n";
      iterOut.flush();
      lastCount = sim.totalChargesAccepted();
    }
    // Now to output the result of the simulation at this data point
    out << i
        << par.temperatureKelvin
        << par.voltageSource
        << par.voltageDrain
        << par.defectPercentage * 100.0
        << par.trapPercentage * 100.0
        << par.chargePercentage * 100.0
        << double(sim.charges()) / double(sim.getMaxCharges()) * 100.0
        << double(lastCount-startCount) / par.iterationsReal << "\n";
    out.flush();

    iterFile.close();

    if ( !par.iterationsXYZ ) 
     {
      trajFile.remove();
     }
    else
     {
      trajFile.close();
     }

  qDebug() << "";
  }

  // Close summary file
  outputFile.close();

  // Find end time
  QDateTime endDateTime(QDateTime::currentDateTime());

  // Output start and end time to log file
  log.setFieldWidth(80);
  log.setPadChar('*');
  log << center << "Simulation Summary" << reset << "\n\n";

  log.setPadChar(' ');
  log.setRealNumberPrecision(10);
  log << right << scientific;
  log << "1.  Time" << "\n";

  log << left;
  log.setFieldWidth(20);
  log << "     A.  time.start";
  log.setFieldWidth(40);
  log << right;
  log << startDateTime.toString("hh:mm:ss d MMMM, yyyy");
  log.setFieldWidth(20);
  log << "-" << "\n";

  log << left;
  log.setFieldWidth(20);
  log << "     B.  time.stop";
  log.setFieldWidth(40);
  log << right;
  log << endDateTime.toString("hh:mm:ss d MMMM, yyyy");
  log.setFieldWidth(20);
  log << "-" << "\n";

  log << left;
  log.setFieldWidth(40);
  log << "     C.  time.elapsed";
  log.setFieldWidth(20);
  log << right;
  log << startDateTime.secsTo(endDateTime) << "sec" << "\n";

  log << "\n";
  log.setFieldWidth(80);
  log.setPadChar('*');
  log << center << "*" << reset << "\n";
  log.flush();

  // Close log file
  logFile.close();
 
  qDebug() << "Simulation complete - destroy our objects...";

}
