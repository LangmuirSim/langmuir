#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "langmuirviewer.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QStatusBar>
#include <QAction>
#include <QDebug>
#include <QStyle>
#include <QIcon>

MainWindow::MainWindow(const QString &inputFile, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_viewer = ui->qglwidget;
    init();
    m_viewer->load(inputFile);
}

void MainWindow::init()
{
    setIcon(ui->actionExit, "application-exit", QStyle::SP_DialogCloseButton);
    setIcon(ui->actionStart, "media-playback-start", QStyle::SP_MediaPlay);
    setIcon(ui->actionStop, "media-playback-stop", QStyle::SP_MediaStop);
    setIcon(ui->actionOpen, "document-open", QStyle::SP_DialogOpenButton);
    setIcon(ui->actionShow, "camera-photo", QStyle::SP_ArrowBack);
    setIcon(ui->actionUnload, "", QStyle::SP_DialogDiscardButton);
    setIcon(ui->actionSave, "document-save", QStyle::SP_DialogSaveButton);
    setIcon(ui->actionScreenshot, "camera-photo", QStyle::SP_DialogSaveButton);
    setIcon(ui->actionReset, "", QStyle::SP_BrowserReload);
    setIcon(ui->actionSaveSettings, "document-save", QStyle::SP_DialogSaveButton);
    setIcon(ui->actionLoadSettings, "document-open", QStyle::SP_DialogOpenButton);

    connect(m_viewer, SIGNAL(clearMessage()), ui->statusbar, SLOT(clearMessage()));
    connect(m_viewer, SIGNAL(showMessage(QString,int)), ui->statusbar, SLOT(showMessage(QString,int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionScreenshot_triggered()
{
    m_viewer->pause();
    m_viewer->openSnapshotFormatDialog();
    m_viewer->saveSnapshot(false, false);
}

void MainWindow::on_actionOpen_triggered()
{
    m_viewer->pause();

    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open Input File"), QDir::currentPath(), "Simulation (*.inp *.chk);; All Files (*)");

    m_viewer->load(fileName);
}

void MainWindow::on_actionSave_triggered()
{
    m_viewer->pause();

    QString fileName = QFileDialog::getSaveFileName(this, "Save simulation checkpoint",
        QDir::currentPath(), "Simulation (*.inp *.chk);; All Files (*)");

    if (!fileName.isEmpty()) {

        QFileInfo info(fileName);

        if (!(info.suffix() == "inp" || info.suffix() == "chk"))
        {
            QMessageBox::StandardButton choice = QMessageBox::warning(this, "Langmuir",
                "Change to extension to .chk?", QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);

            switch(choice)
            {
                case QMessageBox::Yes:
                {
                    fileName = info.absoluteDir().absoluteFilePath(info.completeBaseName() + ".chk");
                    break;
                }
                case QMessageBox::No:
                {
                    break;
                }
            }
        }
    }

    m_viewer->save(fileName);
}

void MainWindow::on_actionLoadSettings_triggered()
{
    m_viewer->pause();

    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open settings file"), QDir::currentPath(), "Settings (*.ini);; All Files (*)");

    m_viewer->loadSettings(fileName);
}

void MainWindow::on_actionIterations_triggered()
{
    m_viewer->pause();

    bool ok   = false;
    int value = QInputDialog::getInt(this, "Langmuir", "iterations.print", 1, 1, 100, 1, &ok);

    if (ok && value)
    {
        m_viewer->setIterationsPrint(value);
    }
    else {
        m_viewer->errorMessage("can not set iterations.print!");
    }
}

void MainWindow::on_actionPoints_triggered()
{
    m_viewer->pause();
    m_viewer->errorMessage("no implemented yet");
}

void MainWindow::on_actionSaveSettings_triggered()
{
    m_viewer->pause();

    QString fileName = QFileDialog::getSaveFileName(this, "Save settings file",
        QDir::currentPath(), "Settings (*.ini);; All Files (*)");

    if (!fileName.isEmpty()) {

        QFileInfo info(fileName);

        if (info.suffix() != "ini")
        {
            QMessageBox::StandardButton choice = QMessageBox::warning(this, "Langmuir",
                "Change to extension to .ini?", QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);

            switch(choice)
            {
                case QMessageBox::Yes:
                {
                    fileName = info.absoluteDir().absoluteFilePath(info.completeBaseName() + ".ini");
                    break;
                }
                case QMessageBox::No:
                {
                    break;
                }
            }
        }
    }

    m_viewer->saveSettings(fileName);
}

void MainWindow::setStopEnabled(bool enabled)
{
    ui->actionStart->setDisabled(enabled);
    ui->actionStop->setEnabled(enabled);
}

void MainWindow::writeSettings()
{
    QSettings settings("Langmuir", "LangmuirView");
    m_viewer->setSettings(settings);
}

void MainWindow::readSettings()
{
    QSettings settings("Langmuir", "LangmuirView");
    m_viewer->getSettings(settings);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
//    QMessageBox::StandardButton choice = QMessageBox::warning(this, "Langmuir",
//        "Do you really want to quit?", QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);

//    if (choice == QMessageBox::Yes) {
//        writeSettings();
//        event->accept();
//    } else {
//        event->ignore();
//    }
    event->accept();
}

void MainWindow::setIcon(QAction *action, QString themeIcon, QStyle::StandardPixmap standardPixmap)
{
    if (action == NULL) {
        return;
    }
    if (QIcon::hasThemeIcon(themeIcon)) {
        action->setIcon(QIcon::fromTheme(themeIcon));
    }
    else {
        action->setIcon(this->style()->standardIcon(standardPixmap));
    }
}
