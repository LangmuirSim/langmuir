#ifndef ISOSURFACEDIALOG_H
#define ISOSURFACEDIALOG_H

#include "mesh.h"

#include <QDialog>

namespace Ui {
class IsoSurfaceDialog;
}

namespace LangmuirView {

class LangmuirViewer;

/**
 * @brief A class to get isosurface parameters from the user
 */
class IsoSurfaceDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief create the IsoSurfaceDialog
     * @param viewer the viewer
     * @param parent QObject this belongs to
     */
    explicit IsoSurfaceDialog(LangmuirViewer &viewer, QWidget *parent = 0);

    /**
      * @brief destroy the IsoSurfaceDialog
      */
    ~IsoSurfaceDialog();

public slots:
    /**
     * @brief initialize dialog
     */
    void init();

    /**
     * @brief update dialog
     */
    void update();

    /**
     * @brief start isosurface calculation
     */
    void on_calculateButton_clicked();

    /**
     * @brief allow isosurface calculation
     * @param enabled true if allowed
     */
    void setCalculateEnabled(bool enabled);

    /**
     * @brief set progress bar range
     * @param low lower bound
     * @param high upper bound
     */
    void setProgressRange(int low, int high);

    /**
     * @brief set current value of progress bar
     * @param value current progress
     */
    void setProgress(int value);

    /**
     * @brief update combo box text
     * @param mode selection number
     */
    void updateComboBoxMode(Mesh::Mode mode);

    /**
     * @brief update spin box value
     * @param value value
     */
    void updateSpinBoxAlpha(int value);

    /**
     * @brief set trap mesh render mode
     * @param text mode as string
     */
    void on_comboBoxMode_currentTextChanged(const QString & text);

    /**
     * @brief set trap mesh alpha value
     * @param value value
     */
    void on_spinBoxAlpha_valueChanged(int value);

    /**
     * @brief get alpha component of color and emit it (using sendAlpha)
     * @param color color to analyze
     */
    void extractAlpha(QColor color);

signals:
    /**
     * @brief sendAlpha
     * @param value
     */
    void sendAlpha(int value);

private:
    //! dialog UI
    Ui::IsoSurfaceDialog *ui;

    //! reference to OpenGL widget
    LangmuirViewer& m_viewer;
};

}

#endif // ISOSURFACEDIALOG_H
