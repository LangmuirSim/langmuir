#include <iostream>
#include <unistd.h>

#include "cubicgrid.h"
#include "simulation.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

using namespace std;
using namespace Langmuir;

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
  cout << "Testing cubic grid class...\n";
  CubicGrid test(10, 10);

  int site1 = 5;
  int site2 = 66;

  qDebug() << "Distance between sites:" << test.getTotalDistance(site1, site2)
           << "," << test.getXDistance(site1, site2) << ","
           << test.getYDistance(site1, site2) << "\n";

  // Create a 16x16 grid with a source potential of -5 and a drain potential of 0
  qDebug() << "Testing the simulation class.\n";
  Simulation sim(10, 5, 0, -0.01);

  for (int i = 0; i < 100000; i++) {
    sim.printGrid();
    sim.performIterations(1);
    usleep(10000);
  }
  qDebug() << "Simulation complete - destroy our objects...";
}
