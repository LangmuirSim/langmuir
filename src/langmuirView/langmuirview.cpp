#include <QApplication>
#include "mainwindow.h"
#include "clparser.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QStringList args = app.arguments();
    LangmuirCore::CommandLineParser clparser;
    clparser.setDescription("View charge transport simulation.");
    clparser.addPositional("input", "input file name");
    clparser.parse(args);

    QString inputFile = clparser.get<QString>("input", "");
    LangmuirView::MainWindow window(inputFile);
    window.show();

    return app.exec();
}
