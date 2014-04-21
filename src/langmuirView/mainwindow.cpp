#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "langmuirviewer.h"

#include <QAction>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_viewer = ui->qglwidget;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::toggleStartStopStatus()
{
    if (ui->actionStart->isEnabled()) {
        ui->actionStart->setEnabled(false);
        ui->actionStop->setDisabled(false);
    }
    else {
        ui->actionStart->setEnabled(true);
        ui->actionStop->setDisabled(true);
    }
}
