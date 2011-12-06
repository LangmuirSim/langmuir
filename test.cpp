#include <QtCore>
#include "inputparser.h"
#include "solar.h"

using namespace Langmuir;
int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
  //InputParserTemp parser("/home/adam/Desktop/adam.inp");
  SimulationParameters par;
  SolarCell cell(&par,0);

  return 0;
  double     D =   5;
  double   *pD =  &D;
  void     *vD =  &D;
  double   *vp = (double*)vD;

  qDebug() << "  value( object):" <<   D;
  qDebug() << "  value(pointer):" << *pD;
  qDebug() << "address( object):" <<  &D;
  qDebug() << "address(pointer):" <<  pD;
  qDebug() << "address(   void):" <<  vD;
  qDebug() << "address(   cast):" <<  vp;
  qDebug() << "";

  *vp = 7;

  qDebug() << "  value( object):" <<   D;
  qDebug() << "  value(pointer):" << *pD;
  qDebug() << "address( object):" <<  &D;
  qDebug() << "address(pointer):" <<  pD;
  qDebug() << "address(   void):" <<  vD;
  qDebug() << "address(   cast):" <<  vp;
  qDebug() << "";
}
