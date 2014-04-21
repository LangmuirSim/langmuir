#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class LangmuirViewer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void toggleStartStopStatus();

private:
    Ui::MainWindow *ui;
    LangmuirViewer *m_viewer;
};

#endif // MAINWINDOW_H
