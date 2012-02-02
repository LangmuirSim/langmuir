#include <QtCore>
#include "inputparser.h"
#include "logger.h"
#include "world.h"

using namespace Langmuir;
int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  InputParser parser("lvsample.inp");
  SimulationParameters &par = parser.getParameters(0);
  World world(&par);
}
