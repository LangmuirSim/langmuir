#ifndef POINTDIALOG_H
#define POINTDIALOG_H

#include "pointcloud.h"

#include <QDialog>

namespace Ui {
class PointDialog;
}

namespace LangmuirView {

class LangmuirViewer;

/**
 * @brief A class to get point cloud properties from the user
 */
class PointDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief create the PointDialog
     * @param viewer the viewer
     * @param parent QObject this belongs to
     */
    explicit PointDialog(LangmuirViewer &viewer, QWidget *parent = 0);

    /**
      * @brief destroy the PointDialog
      */
    ~PointDialog();

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
     * @brief remember settings (for undo)
     */
    void remember();

    /**
     * @brief undo settings
     */
    void reset();

    /**
     * @brief update combo box text
     * @param mode selection number
     */
    void updateComboBoxElectrons(PointCloud::Mode mode);

    /**
     * @brief update combo box text
     * @param mode selection number
     */
    void updateComboBoxDefects(PointCloud::Mode mode);

    /**
     * @brief update combo box text
     * @param mode selection number
     */
    void updateComboBoxHoles(PointCloud::Mode mode);

    /**
     * @brief update combo box text
     * @param mode selection number
     */
    void updateComboBoxTraps(PointCloud::Mode mode);

    /**
     * @brief update spin box value
     * @param d value
     */
    void updateSpinBoxElectrons(float d);

    /**
     * @brief update spin box value
     * @param d value
     */
    void updateSpinBoxDefects(float d);

    /**
     * @brief update spin box value
     * @param d value
     */
    void updateSpinBoxHoles(float d);

    /**
     * @brief update spin box value
     * @param d value
     */
    void updateSpinBoxTraps(float d);

    /**
     * @brief update check box state
     * @param checked true if checked
     */
    void updateCheckBoxElectrons(bool checked);

    /**
     * @brief update check box state
     * @param checked true if checked
     */
    void updateCheckBoxDefects(bool checked);

    /**
     * @brief update check box state
     * @param checked true if checked
     */
    void updateCheckBoxHoles(bool checked);

    /**
     * @brief update check box state
     * @param checked true if checked
     */
    void updateCheckBoxTraps(bool checked);

    /**
     * @brief set pointcloud render mode
     * @param text mode as string
     */
    void on_comboBoxElectrons_currentTextChanged(const QString & text);

    /**
     * @brief set pointcloud render mode
     * @param text mode as string
     */
    void on_comboBoxDefects_currentTextChanged(const QString & text);

    /**
     * @brief set pointcloud render mode
     * @param text mode as string
     */
    void on_comboBoxHoles_currentTextChanged(const QString & text);

    /**
     * @brief set pointcloud render mode
     * @param text mode as string
     */
    void on_comboBoxTraps_currentTextChanged(const QString & text);

    /**
     * @brief set pointcloud point size
     * @param d point size
     */
    void on_spinBoxElectrons_valueChanged(double d);

    /**
     * @brief set pointcloud point size
     * @param d point size
     */
    void on_spinBoxDefects_valueChanged(double d);

    /**
     * @brief set pointcloud point size
     * @param d point size
     */
    void on_spinBoxHoles_valueChanged(double d);

    /**
     * @brief set pointcloud point size
     * @param d point size
     */
    void on_spinBoxTraps_valueChanged(double d);

    /**
     * @brief set pointcloud visible state
     * @param state visible state
     */
    void on_checkBoxElectrons_stateChanged(int state);

    /**
     * @brief set pointcloud visible state
     * @param state visible state
     */
    void on_checkBoxDefects_stateChanged(int state);

    /**
     * @brief set pointcloud visible state
     * @param state visible state
     */
    void on_checkBoxHoles_stateChanged(int state);

    /**
     * @brief set pointcloud visible state
     * @param state visible state
     */
    void on_checkBoxTraps_stateChanged(int state);

    /**
     * @brief reset values
     */
    void on_pushButtonReset_clicked();

    /**
     * @brief reset values
     */
    void on_buttonBox_rejected();

private:
    //! dialog UI
    Ui::PointDialog *ui;

    //! reference to OpenGL widget
    LangmuirViewer& m_viewer;

    //! old pointSize (for undo)
    float e_pointSize_old;

    //! old pointSize (for undo)
    float d_pointSize_old;

    //! old pointSize (for undo)
    float h_pointSize_old;

    //! old pointSize (for undo)
    float t_pointSize_old;

    //! old mode (for undo)
    PointCloud::Mode e_mode_old;

    //! old mode (for undo)
    PointCloud::Mode d_mode_old;

    //! old mode (for undo)
    PointCloud::Mode h_mode_old;

    //! old mode (for undo)
    PointCloud::Mode t_mode_old;

    //! old visible state (for undo)
    bool e_visible;

    //! old visible state (for undo)
    bool d_visible;

    //! old visible state (for undo)
    bool h_visible;

    //! old visible state (for undo)
    bool t_visible;
};

}

#endif // POINTDIALOG_H
