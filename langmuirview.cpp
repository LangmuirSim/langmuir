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
        QStringList options;
        options << QObject::tr("choose input file")
                << QObject::tr("LangmuirView::PredefinedParameters::Default")
                << QObject::tr("LangmuirView::PredefinedParameters::NeutralDefects")
                << QObject::tr("LangmuirView::PredefinedParameters::HomogeneousTraps")
                << QObject::tr("LangmuirView::PredefinedParameters::HeterogeneousTraps");
        bool ok = false;
        QString item = QInputDialog::getItem(0, QObject::tr("Langmuir"), QObject::tr("Langmuir requires an input file!"), options, 0, false, &ok);
        if (ok && !item.isEmpty())
        {
            switch( options.indexOf(item) )
            {
            case 0:
            {
                iFileName = QFileDialog::getOpenFileName( 0, QObject::tr("Input File Name"), QDir::currentPath() );
                if ( iFileName.isEmpty() )
                {
                    QMessageBox::critical(0,QObject::tr("Langmuir"),QObject::tr("No input file chosen!"));
                }
                break;
            }
            default:
            {
                iFileName = item;
                break;
            }
            }
        }
        else
        {
           return 0;
        }
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
