#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "langmuirviewer.h"

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
        this, tr("Open Input File"), QDir::currentPath());

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
        m_viewer->save(fileName);
    }
}

void MainWindow::setStopEnabled(bool enabled)
{
    ui->actionStart->setDisabled(enabled);
    ui->actionStop->setEnabled(enabled);
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
