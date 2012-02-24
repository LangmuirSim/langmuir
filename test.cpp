#include <QtCore>

#include "parameters.h"
#include "output.h"

using namespace Langmuir;
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    SimulationParameters par;

    FileNameGenerator gen(par);
    qDebug() << qPrintable(gen.generate("coulomb","dat",FileNameGenerator::IgnorePath));
    //= FileNameGenerator::UseStep|FileNameGenerator::UseSimulation;
}
