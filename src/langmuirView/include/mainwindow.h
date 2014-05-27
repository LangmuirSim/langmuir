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
class IsoSurfaceDialog;

/**
 * @brief A window with an OpenGL widget
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString& inputFile="", QWidget *parent = 0);
    ~MainWindow();

public slots:
    //! take a screen shot
    void on_actionScreenshot_triggered();

    //! open an input file
    void on_actionOpen_triggered();

    //! save checkpoint file
    void on_actionSave_triggered();

    //! reset settings (looks for config.ini)
    void on_actionResetSettings_triggered();

    //! load settings file
    void on_actionLoadSettings_triggered();

    //! save settings file
    void on_actionSaveSettings_triggered();

    //! open point dialog
    void on_actionPoints_triggered();

    //! open isosurface dialog
    void on_actionIsoSurface_triggered();

    //! open checker dialog
    void on_actionChecker_triggered();

    //! enabled or disable stop and play buttons
    void setStopEnabled(bool enabled);

    //! clean up before closing
    void closeEvent(QCloseEvent *event);

    //! setup some connections that require OpenGL to be initialized first
    void initAfter();

    //! update the iterations.print spin box
    void updateSpinBox(int value);

private:
    //! main window user interface
    Ui::MainWindow *ui;

    //! OpenGL widget
    LangmuirViewer *m_viewer;

    //! dialog to set point parameters
    PointDialog *m_pointdialog;

    //! dialog to create isosurface
    IsoSurfaceDialog *m_isosurfacedialog;

    //! set user interface icon
    void setIcon(QAction *action, QString themeIcon, QStyle::StandardPixmap standardPixmap);

    //! setup
    void init();
};

#endif // MAINWINDOW_H
