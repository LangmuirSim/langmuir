#ifndef POINTDIALOG_H
#define POINTDIALOG_H

#include "pointcloud.h"

#include <QDialog>

namespace Ui {
class PointDialog;
}

class LangmuirViewer;

class PointDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PointDialog(LangmuirViewer &viewer, QWidget *parent = 0);
    ~PointDialog();

public slots:
    void init();
    void update();
    void remember();
    void reset();

    void updateComboBoxElectrons(PointCloud::Mode mode);
    void updateComboBoxDefects(PointCloud::Mode mode);
    void updateComboBoxHoles(PointCloud::Mode mode);

    void updateSpinBoxElectrons(float d);
    void updateSpinBoxDefects(float d);
    void updateSpinBoxHoles(float d);

    void updateCheckBoxElectrons(bool checked);
    void updateCheckBoxDefects(bool checked);
    void updateCheckBoxHoles(bool checked);

    void on_comboBoxElectrons_currentTextChanged(const QString & text);
    void on_comboBoxDefects_currentTextChanged(const QString & text);
    void on_comboBoxHoles_currentTextChanged(const QString & text);

    void on_spinBoxElectrons_valueChanged(double d);
    void on_spinBoxDefects_valueChanged(double d);
    void on_spinBoxHoles_valueChanged(double d);

    void on_checkBoxElectrons_stateChanged(int state);
    void on_checkBoxDefects_stateChanged(int state);
    void on_checkBoxHoles_stateChanged(int state);

    void on_pushButtonReset_clicked();
    void on_buttonBox_rejected();

private:
    Ui::PointDialog *ui;
    LangmuirViewer& m_viewer;

    float e_pointSize_old;
    float d_pointSize_old;
    float h_pointSize_old;

    PointCloud::Mode e_mode_old;
    PointCloud::Mode d_mode_old;
    PointCloud::Mode h_mode_old;

    bool e_visible;
    bool d_visible;
    bool h_visible;
};

#endif // POINTDIALOG_H
