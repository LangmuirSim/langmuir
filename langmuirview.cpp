#include "gridview.h"
#include <QtGui>

using namespace Langmuir;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    // read command line arguments
    QStringList args = app.arguments();

    QString iFileName = "";
    if(args.size() != 2)
    {
        QStringList options;
        options << QObject::tr("choose input file");
        bool ok = false;
        QString item = QInputDialog::getItem(0, QObject::tr("Langmuir"), QObject::tr("Langmuir requires an input file!"), options, 0, false, &ok);
        if(ok && !item.isEmpty())
        {
            switch(options.indexOf(item))
            {
            case 0:
            {
                iFileName = QFileDialog::getOpenFileName(0, QObject::tr("Input File Name"), QDir::currentPath());
                if(iFileName.isEmpty())
                {
                    QMessageBox::critical(0, QObject::tr("Langmuir"), QObject::tr("No input file chosen!"));
                    return 0;
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
    else
    {
        iFileName = args.at(1);
    }

    MainWindow window(iFileName);

    if((float)(window.width()* window.height()) /
            (float)(QApplication::desktop()->width()*
                    QApplication::desktop()->height()) < 0.75f)
    {
        window.show();
    }
    else
    {
        window.showMaximized();
    }
    return app.exec();
}
