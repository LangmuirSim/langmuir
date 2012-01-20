#include <QtCore>
#include "logger.h"

using namespace Langmuir;
int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  int col1 = 7;
  double col2 = 8.5;

  DataFile dataFile("test.dat");
  dataFile.addColumn(col1,"col1");
  dataFile.addColumn(col2,"col2");
  dataFile.report();

}
