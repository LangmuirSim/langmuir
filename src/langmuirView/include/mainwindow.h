#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QStyle>
#include <QIcon>

namespace Ui {
class MainWindow;
}

class LangmuirViewer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString& inputFile="", QWidget *parent = 0);
    ~MainWindow();

public slots:
    void on_actionOpen_triggered();
    void on_actionStart_triggered();
    void on_actionStop_triggered();
    void on_actionScreenshot_triggered();

private:
    Ui::MainWindow *ui;
    LangmuirViewer *m_viewer;

    void setIcon(QAction *action, QString themeIcon, QStyle::StandardPixmap standardPixmap);
    void init();
};

#endif // MAINWINDOW_H
