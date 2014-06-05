#ifndef ISOSURFACEDIALOG_H
#define ISOSURFACEDIALOG_H

#include "mesh.h"

#include <QDialog>

namespace Ui {
class IsoSurfaceDialog;
}

class LangmuirViewer;

class IsoSurfaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IsoSurfaceDialog(LangmuirViewer &viewer, QWidget *parent = 0);
    ~IsoSurfaceDialog();

public slots:
    void init();
    void update();
    void on_calculateButton_clicked();
    void setCalculateEnabled(bool enabled);
    void setProgressRange(int low, int high);
    void setProgress(int value);

    void updateComboBoxMode(Mesh::Mode mode);
    void updateSpinBoxAlpha(int value);

    void on_comboBoxMode_currentTextChanged(const QString & text);
    void on_spinBoxAlpha_valueChanged(int value);

    void extractAlpha(QColor color);

signals:
    void sendAlpha(int value);

private:
    Ui::IsoSurfaceDialog *ui;
    LangmuirViewer& m_viewer;
};

#endif // ISOSURFACEDIALOG_H
