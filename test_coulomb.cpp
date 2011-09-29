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
  QCoreApplication app(argc, argv);
  QStringList args = app.arguments();

  if (args.size() < 3) {
    qDebug() << "Need atleast 2 arguments - the input and output file names.";
    app.exit(1);
  }

  qDebug() << "Input file:" << args.at(1) << "\nOutput file:" << args.at(2);

  // Open our main output file and get it ready for writing
  QFile output(args.at(2)+".txt");
  if (!output.open(QIODevice::WriteOnly | QIODevice::Text))
    return 1;
  QTextStream out(&output);

  double minOcc = 0.001; // Percent
  double maxOcc = 0.2; // Percent
  int occStep = 20; // Number of steps
  double occStepSize = (maxOcc - minOcc) / occStep;
 
  SimulationParameters par;

  par.voltageSource = 0.0;
  par.voltageDrain = 1.0;
  par.defectPercentage = 0.0;
  par.trapPercentage = 0.0;
  par.chargePercentage = minOcc;
  par.temperatureKelvin = 300.0;
  par.deltaEpsilon = 0.0;
  par.gridWidth = 1024;
  par.gridHeight = 256;
  par.gridDepth = 0;
  par.zDefect = 0;
  par.zTrap = 0;
  par.iterationsWarmup = 0;
  par.iterationsReal = 1000;
  par.iterationsPrint = 100;
  par.iterationsTraj = 100;
  par.coulomb = true;
  par.chargedDefects = false;
  par.chargedTraps = false;
  par.gridCharge = false;
  par.outputXyz = false;
  par.potentialForm = SimulationParameters::o_linearpotential;

  qDebug() << "Testing the simulation class.\n";

  for (int i = 0; i < occStep; ++i) {
    par.chargePercentage = minOcc + i*occStepSize;
    Simulation *sim = new Simulation(&par);

    int charges = par.gridWidth * par.gridHeight * (minOcc + i*occStepSize);

    qDebug() << "Starting simulation run...\nCoulomb: true\nCharges:" << charges << "\tOccupation:" << double(charges) / (par.gridWidth*par.gridHeight);

    QFile output2(args.at(2)+QString::number(i)+".txt");
    if (!output2.open(QIODevice::WriteOnly | QIODevice::Text))
     return 1;
    QTextStream out2(&output2);
    out2 << "Charges: " << charges << "\tOccupation: " << double(charges) / (par.gridWidth*par.gridHeight) << "\n\n";

    unsigned long lastCount = 0;
    for (int j = 0; j < 25; ++j) {
      sim->performIterations(10000);
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
    for (int j = 0; j < 100; ++j) {
      sim->performIterations(10000);
      qDebug() << "Real:\t" << j << sim->totalChargesAccepted() - lastCount << sim->charges() << "of" << charges;
      out2 << j << "\t" << sim->totalChargesAccepted() - lastCount << "\t" << double(sim->charges()) / charges << "\n";
      out2.flush();
      lastCount = sim->totalChargesAccepted();
    }
    out2 << "\n";
    output2.close();

    // Now to write the running total across runs
    out << i << "\t" << charges << "\t" << lastCount - startCount << "\n";
    out.flush();

    // Clean up old simulation
    delete sim;
    sim = 0;
  }

  output.close();
}
