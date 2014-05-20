#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QMainWindow>
#include <QAction>
#include <QStyle>
#include <QIcon>

namespace Ui {
class MainWindow;
}

class LangmuirViewer;
class PointDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString& inputFile="", QWidget *parent = 0);
    ~MainWindow();

public slots:
    void on_actionScreenshot_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionResetSettings_triggered();
    void on_actionLoadSettings_triggered();
    void on_actionSaveSettings_triggered();
    void on_actionIterations_triggered();
    void on_actionPoints_triggered();
    void on_actionIsoSurface_triggered();
    void setStopEnabled(bool enabled);
    void writeSettings();
    void readSettings();
    void closeEvent(QCloseEvent *event);
    void initAfter();

private:
    Ui::MainWindow *ui;
    LangmuirViewer *m_viewer;
    PointDialog *m_pointdialog;

    void setIcon(QAction *action, QString themeIcon, QStyle::StandardPixmap standardPixmap);
    void init();
};

#endif // MAINWINDOW_H
