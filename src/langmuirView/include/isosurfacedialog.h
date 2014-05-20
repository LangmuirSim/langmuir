#ifndef ISOSURFACEDIALOG_H
#define ISOSURFACEDIALOG_H

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

private:
    Ui::IsoSurfaceDialog *ui;
    LangmuirViewer& m_viewer;
};

#endif // ISOSURFACEDIALOG_H
