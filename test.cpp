#include <iostream>
#include <unistd.h>

#include "cubicgrid.h"
#include "simulation.h"
#include "inputparser.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

using namespace std;
using namespace Langmuir;

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  cout << "Testing cubic grid class...\n";
  
  int width  = 2; //The number of columns
  int height = 2; //The number of rows
  int depth  = 2; //The number of layers
 
  cout << "Creating CubicGrid: "                  << "\n";
  CubicGrid test(width,height,depth);

  cout << "             width: " << test.width()  << "\n";
  cout << "            height: " << test.height() << "\n";
  cout << "             depth: " << test.depth()  << "\n"; 
  cout << "              area: " << test.area()   << "\n";
  cout << "            volume: " << test.volume() << "\n";

  cout << "Calculating site IDs..." << "\n";
  for ( int k = 0; k <  depth; k++ ) { //loop over layers
  for ( int j = 0; j < height; j++ ) { //loop over rows
  for ( int i = 0; i <  width; i++ ) { //loop over collumns

   int ID = test.getIndex(i,j,k);
   int  c = test.getColumn(ID);
   int  r = test.getRow(ID);
   int  l = test.getLayer(ID);
   cout << "ON SITE: ( ";
   cout.width(4);
   cout << i << " ";
   cout.width(4);
   cout << j << " ";
   cout.width(4);
   cout << k << " ) SITE ID CALCULATION: "; 
   cout.width(4);
   cout << ID;
   cout << " BACKWARDS CALCULATION: ( ";
   cout.width(4);
   cout << c << " ";
   cout.width(4);
   cout << r << " ";
   cout.width(4);
   cout << l << " )";
   cout << "\n"; 
  }
  }
  }

  QFile outputFile("test.xyz");
  QTextStream out(&outputFile);
  if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qDebug() << "Error opening output file:" << "test.dat";
    return -1;
  }

  SimulationParameters par;

  par.voltageSource = 0.0;
  par.voltageDrain = 1.0;
  par.defectPercentage = 0.0;
  par.trapPercentage = 0.0;
  par.chargePercentage = 0.0;
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
  par.coulomb = false;
  par.chargedDefects = false;
  par.chargedTraps = false;
  par.gridCharge = false;
  par.outputXyz = false;
  par.potentialForm = SimulationParameters::o_linearpotential;

  Simulation sim(&par);
  for (int i = 0; i < 1000; i++) {
    qDebug() << "Step: " << i << "Accepted charges:" << sim.totalChargesAccepted();
    sim.performIterations(1);
    sim.getGrid()->print3D(out);
    usleep(100);
  }
  outputFile.close();
  qDebug() << "Simulation complete - destroy our objects...";
}
