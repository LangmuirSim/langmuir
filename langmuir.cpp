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
  if (args.size() < 3) {
    qDebug() << "Need atleast 1 argument - the input and output file names.";
    app.exit(1);
  }

  QString inputFileName = args.at(1);
  QString outputFileName = args.size() > 2 ? args.at(2) : args.at(1);

  // Now open our input file
  QFile inputFile(inputFileName);
  if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Error opening intput file:" << inputFileName;
    app.exit(1);
  }
  inputFile.close();
  InputParser input(inputFileName);
  SimulationParameters par;
  input.simulationParameters(&par);

  // Open our main output file and get it ready for writing
  QFile outputFile(outputFileName+".dat");
  if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qDebug() << "Error opening output file:" << outputFileName + ".dat";
    app.exit(1);
  }
  QTextStream out(&outputFile);

  QDateTime startDateTime(QDateTime::currentDateTime());
  // Output a summary of all simulation parameters
  out << "Langmuir Simulation Code\nHutchison Group\n\n"
      << "Started: " << startDateTime.toString("hh:mm:ss d MMMM, yyyy")
      << "\n\nvariable.working: " << input.workingVariable()
      << "\nvariable.start: " << input.start()
      << "\nvariable.final: " << input.final()
      << "\nvariable.steps: " << input.steps()
      << "\niterations.warmup: " << par.iterationsWarmup
      << "\niterations.real: " << par.iterationsReal
      << "\niterations.print: " << par.iterationsPrint
      << "\ngrid.width: " << par.gridWidth
      << "\ngrid.height: " << par.gridHeight
      << "\ngrid.charge: " << par.gridCharge
      << "\ninteractions.coulomb: " << par.coulomb
      << "\nvoltage.source: " << par.voltageSource
      << "\nvoltage.drain: " << par.voltageDrain
      << "\ntrap.percentage: " << par.trapPercentage * 100.0
      << "\ncharge.percentage: " << par.chargePercentage * 100.0
      << "\ntemperature.kelvin: " << par.temperatureKelvin
      << "\n\n";

  // Now output the column titles
  out << "#i\tTemperature (K)\tSource (V)\tDrain (V)\tTrap (%)\tCharge Goal (%)"
      << "\tCharge Reached (%)\tCharge Transport (per iteration)\n";
  out.flush();

  for (int i = 0; i < input.steps(); ++i) {
    // Get simulation parameters for the current step and set up a new object
    input.simulationParameters(&par, i);
    qDebug() << i
        << "\nvoltage.source:" << par.voltageSource
        << "\nvoltage.drain:" << par.voltageDrain
        << "\ntrap.percentage:" << par.trapPercentage;
    Simulation sim(par.gridWidth, par.gridHeight,
                   par.voltageSource, par.voltageDrain,
                   par.trapPercentage);
    int nCharges = par.chargePercentage * double(par.gridWidth*par.gridHeight);
    sim.setMaxCharges(nCharges);
    sim.setCoulombInteractions(par.coulomb);

    // Charge the grid up is specified
    if (par.gridCharge)
      sim.seedCharges();

    // Now to start the simulation - warmup and then real counts
    QFile iterFile(outputFileName+"-i-"+QString::number(i)+".dat");
    if (!iterFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
      qDebug() << "Error opening output file:"
          << outputFileName+"-i-"+QString::number(i)+".dat";
      app.exit(1);
    }
    QTextStream iterOut(&iterFile);
    iterOut << "#i\tSource (V)\tDrain (V)\tTrap (%)\tCharge Goal (%)"
            << "\tCharge Reached (%)\tCharge Transport (per iteration)\n";
    unsigned long lastCount = 0;
    for (int j = 0; j < par.iterationsWarmup; j += par.iterationsPrint) {
      sim.performIterations(par.iterationsPrint);
      // Now to output the result of the simulation at this data point
      qDebug() << j
               << "\t" << double(sim.charges()) / double(nCharges) * 100.0
               << "\t" << double(sim.totalChargesAccepted()-lastCount) / par.iterationsPrint;
      iterOut << j
              << "\t" << par.voltageSource
              << "\t" << par.voltageDrain
              << "\t" << par.trapPercentage * 100.0
              << "\t" << par.chargePercentage * 100.0
              << "\t" << double(sim.charges()) / double(nCharges) * 100.0
              << "\t" << double(sim.totalChargesAccepted()-lastCount) / par.iterationsPrint << "\n";
      iterOut.flush();
      lastCount = sim.totalChargesAccepted();
    }
    unsigned long startCount = lastCount;
    for (int j = 0; j < par.iterationsReal; j += par.iterationsPrint) {
      sim.performIterations(par.iterationsPrint);
      // Now to output the result of the simulation at this data point
      qDebug() << j
               << "\t" << double(sim.charges()) / double(nCharges) * 100.0
               << "\t" << double(sim.totalChargesAccepted()-lastCount) / par.iterationsPrint;
      iterOut << j
              << "\t" << par.voltageSource
              << "\t" << par.voltageDrain
              << "\t" << par.trapPercentage * 100.0
              << "\t" << par.chargePercentage * 100.0
              << "\t" << double(sim.charges()) / double(nCharges) * 100.0
              << "\t" << double(sim.totalChargesAccepted()-lastCount) / par.iterationsPrint << "\n";
      iterOut.flush();
      lastCount = sim.totalChargesAccepted();
    }
    // Now to output the result of the simulation at this data point
    out << i
        << "\t" << par.voltageSource
        << "\t" << par.voltageDrain
        << "\t" << par.trapPercentage * 100.0
        << "\t" << par.chargePercentage * 100.0
        << "\t" << double(sim.charges()) / double(nCharges) * 100.0
        << "\t" << double(lastCount-startCount) / par.iterationsReal << "\n";
    out.flush();
  }

  outputFile.close();
  qDebug() << "Simulation complete - destroy our objects...";

}
