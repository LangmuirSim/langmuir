#include "gridview.h"
#include <QtGui>

using namespace Langmuir;

int main (int argc, char **argv)
{
    QApplication app (argc, argv);

    // read command line arguments
    QStringList args = app.arguments ();

    QString iFileName = "";
    QString oFileName = "";
    if ( args.size() == 2 )
    {
        iFileName = args.at(1);
        QStringList tokens = args.at(1).split("/",QString::SkipEmptyParts);
        oFileName = tokens[tokens.size()-1].split(".",QString::SkipEmptyParts)[0];
    }
    else if ( args.size() == 3 )
    {
        iFileName = args.at(1);
        oFileName = args.at(2);
    }
    else
    {
        QMessageBox::critical(0, "Langmuir", "correct use is:\n\n\tlangmuirView input.dat (output.dat)\t\n", QMessageBox::Ok);
        qFatal("bad input");
    }

    MainWindow window( iFileName );

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
