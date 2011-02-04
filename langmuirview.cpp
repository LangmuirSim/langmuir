#include "gridview.h"
#include <QtGui>

using namespace Langmuir;

int
main (int argc, char **argv)
{
  QApplication app (argc, argv);

  MainWindow window;
  window.resize (window.sizeHint ());

  if ((float) (window.width () * window.height ()) /
      (float) (QApplication::desktop ()->width () *
           QApplication::desktop ()->height ()) < 0.75f)
    {
      window.show ();
    }
  else
    {
      window.showMaximized ();
    }
  return app.exec ();
}
