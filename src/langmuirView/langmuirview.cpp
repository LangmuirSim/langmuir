/**
  * @file langmuirview.cpp
  * @brief # The langmuirView executable.
  *
  * Here we find the main LangmuirView function, meant to run simulations interactivly and visualize them in real time.
  */
#include <QApplication>
#include "mainwindow.h"
#include "clparser.h"

/**
 * @brief main function.
 * @param argc number of command line arguments
 * @param argv vector of command line arguments
 */
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
