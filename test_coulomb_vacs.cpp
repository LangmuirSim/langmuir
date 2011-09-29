#include <iostream>
#include <unistd.h>

#include "cubicgrid.h"
#include "simulation.h"
#include "inputparser.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

using namespace std;
using namespace Langmuir;

int main(int argc, char *argv[])
{

  double minTrap = 0.00;

  QCoreApplication app(argc, argv);
  QStringList args = app.arguments();

  if (args.size() < 3) {
    qDebug() << "Need atleast 2 arguments - the input and output file names.";
    app.exit(1);
  }

  if (args.size() > 2) {
    qDebug() << "Setting the trap concentration:" << args.at(3);
    minTrap = QString(args.at(3)).toDouble();
  }

  int its = 250000;
  if (args.size() > 3) {
    qDebug() << "Setting the number of iterations:" << args.at(4);
    its = args.at(4).toInt();
  }
  // We output the numbers 50 times in a run, so divide the desired its by that
  its /= 50;
  qDebug() << "Iterations to be used between output:" << its;

  qDebug() << "Input file:" << args.at(1) << "\nOutput file:" << args.at(2);

  // Open our main output file and get it ready for writing
  QFile output(args.at(2)+".txt");
  if (!output.open(QIODevice::WriteOnly | QIODevice::Text))
    return 1;
  QTextStream out(&output);

  double maxTrap = 0.30; // Percent
  double occupation = 0.01; // 1 percent constant charge occupation
  int step = 4; // Number of steps
  double stepSize = (maxTrap - minTrap) / (step-1);
  stepSize = 0.0;

  SimulationParameters par;

  par.voltageSource = 0.0;
  par.voltageDrain = 1.0;
  par.defectPercentage = 0.0;
  par.trapPercentage = minTrap;
  par.chargePercentage =  occupation;
  par.temperatureKelvin = 300.0;
  par.deltaEpsilon = 0.0;
  par.gridWidth = 1024;
  par.gridHeight = 256;
  par.gridDepth = 0;
  par.zDefect = 0;
  par.zTrap = 0;
  par.iterationsWarmup = 0;
  par.iterationsReal = step;
  par.iterationsPrint = 100;
  par.iterationsTraj = 100;
  par.coulomb = true;
  par.chargedDefects = false;
  par.chargedTraps = false;
  par.gridCharge = false;
  par.outputXyz = false;
  par.potentialForm = SimulationParameters::o_linearpotential;

  qDebug() << "Testing the simulation class.\n";

  for (int i = 0; i < step; ++i) {
    double trapPercent = minTrap + i * stepSize;
    par.trapPercentage = trapPercent;
    Simulation *sim = new Simulation(&par);
    int charges = par.gridWidth * par.gridHeight * occupation;

    qDebug() << "Starting simulation run...\nCoulomb: true\nCharges:"
        << charges << "\tOccupation:" << double(charges) / (par.gridWidth*par.gridHeight)
        << "\tDrain Voltage:" << par.voltageDrain
        << "\tTraps: " << trapPercent;

    QFile output2(args.at(2)+QString::number(i)+".txt");
    if (!output2.open(QIODevice::WriteOnly | QIODevice::Text))
      return 1;
    QTextStream out2(&output2);
    out2 << "Charges: " << charges << "\tOccupation: "
        << double(charges) / (par.gridWidth*par.gridHeight)
        << "\tSource: " << par.voltageSource
        << "\tDrain: " << par.voltageDrain
        << "\tTraps: " << trapPercent << "\n\n";

    unsigned long lastCount = 0;
    for (int j = 0; j < 25; ++j) {
      sim->performIterations(5000);
      qDebug() << "Warmup:\t" << j << "\t" << sim->totalChargesAccepted() - lastCount
          << sim->charges() << "of" << charges;
      out2 << j << "\t" << sim->totalChargesAccepted() - lastCount
          << "\t" << double(sim->charges()) / charges
          << "\n";
      out2.flush();
      lastCount = sim->totalChargesAccepted();
    }

    // Now for the real work
    int startCount = lastCount;
    for (int j = 0; j < 50; ++j) {
      sim->performIterations(its);
      qDebug() << "Real:\t" << j << sim->totalChargesAccepted() - lastCount
          << sim->charges() << "of" << charges;
      out2 << j << "\t" << sim->totalChargesAccepted() - lastCount
          << "\t" << double(sim->charges()) / charges
          << "\n";
      out2.flush();
      lastCount = sim->totalChargesAccepted();
    }
    out2 << "\n";
    output2.close();

    // Now to write the running total across runs
    out << i << "\t" << trapPercent<< "\t"
        << charges << "\t" << lastCount - startCount << "\n";
    out.flush();

    // Clean up old simulation
    delete sim;
    sim = 0;
  }

  output.close();
}
