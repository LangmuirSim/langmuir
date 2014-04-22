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
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void toggleStartStopStatus();

private:
    Ui::MainWindow *ui;
    LangmuirViewer *m_viewer;

    void setIcon(QAction *action, QString themeIcon, QStyle::StandardPixmap standardPixmap);
    void init();
};

#endif // MAINWINDOW_H
