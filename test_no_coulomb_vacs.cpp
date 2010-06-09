#include <iostream>
#include <unistd.h>

#include "cubicgrid.h"
#include "simulation.h"

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

  int width = 1024;
  int height = 256;
  int depth = 1;
  double sourceVoltage = 0.0;
  double drainVoltage = 1.0;
  double minTrap = 0.12; // Percent
  double maxTrap = 0.18; // Percent
  double occupation = 0.01; // 1 percent constant charge occupation
  int step = 4; // Number of steps
  double stepSize = (maxTrap - minTrap) / (step-1);

  qDebug() << "Testing the simulation class.\n";

  for (int i = 0; i < step; ++i) {
    double trapPercent = minTrap + i * stepSize;
    Simulation *sim = new Simulation(width, height, depth, sourceVoltage, drainVoltage,
                                     trapPercent);
    int charges = width * height * occupation;
    sim->setMaxCharges(charges);
    sim->setCoulombInteractions(false);

    qDebug() << "Starting simulation run...\nCoulomb: false\nCharges:"
        << charges << "\tOccupation:" << double(charges) / (width*height)
        << "\tDrain Voltage:" << drainVoltage
        << "\tTraps: " << trapPercent;

    QFile output2(args.at(2)+QString::number(i)+".txt");
    if (!output2.open(QIODevice::WriteOnly | QIODevice::Text))
      return 1;
    QTextStream out2(&output2);
    out2 << "Charges: " << charges << "\tOccupation: "
        << double(charges) / (width*height)
        << "\tSource: " << sourceVoltage
        << "\tDrain: " << drainVoltage
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
    unsigned int startCount = lastCount;
    for (int j = 0; j < 50; ++j) {
      sim->performIterations(5000);
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
