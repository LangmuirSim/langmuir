#ifndef LANGMUIRVIEWER_H
#define LANGMUIRVIEWER_H

#include <QGLViewer/qglviewer.h>
#include <QErrorMessage>
#include <QColorDialog>
#include <QMatrix4x4>
#include <QSettings>

#include "corneraxis.h"
#include "pointcloud.h"
#include "light.h"
#include "grid.h"
#include "rand.h"
#include "mesh.h"
#include "box.h"

namespace Langmuir {
    class Simulation;
    class World;
}

/**
 * @brief Widget to view Langmuir Simulation in real time.
 */
class LangmuirViewer : public QGLViewer
{
    Q_OBJECT
public:
    /**
     * @brief create the LangmuirViewer
     * @param parent QObject this belongs to
     */
    explicit LangmuirViewer(QWidget *parent = 0);

    /**
      * @brief destroy the LangmuirViewer
      */
    ~LangmuirViewer();

    /**
     * @brief obtain the model-view-projection matrix (QGLViewer camera)
     */
    QMatrix4x4& getModelViewProjectionMatrix();

    /**
     * @brief obtain the projection matrix (QGLViewer camera)
     */
    QMatrix4x4& getProjectionMatrix();

    /**
     * @brief obtain the model-view matrix (OpenGL matrix stack)
     */
    QMatrix4x4& getOpenGLModelViewMatrix();

    /**
     * @brief obtain the projection matrix (OpenGL matrix stack)
     */
    QMatrix4x4& getOpenGLProjectionMatrix();

    /**
     * @brief get the random number generator
     */
    Langmuir::Random& random();

    /**
     * @brief get the trap color
     */
    const QColor& trapColor() const { return m_trapColor; }

    /**
     * @brief get corner axis object
     */
    CornerAxis & cornerAxis() { return *m_cornerAxis; }

    /**
     * @brief get electrons object
     */
    PointCloud & electrons() { return *m_electrons; }

    /**
     * @brief get defects object
     */
    PointCloud & defects() { return *m_defects; }

    /**
     * @brief get holes object
     */
    PointCloud & holes() { return *m_holes; }

    /**
     * @brief get right box object
     */
    Box & rightBox() { return *m_rBox; }

    /**
     * @brief get left box object
     */
    Box & leftBox() { return *m_lBox; }

    /**
     * @brief get base box object
     */
    Box & baseBox() { return *m_baseBox; }

    /**
     * @brief get trap box object
     */
    Box & trapBox() { return *m_trapBox; }

    /**
     * @brief get light object
     */
    Light & light() { return *m_light0; }

    /**
     * @brief get grid object
     */
    Grid & grid() { return *m_grid; }

signals:
    /**
     * @brief show a message in the status bar
     * @param message string to display
     * @param timeout message display time in ms
     */
    void showMessage(const QString& message, int timeout=0);

    /**
     * @brief clear the status bar message
     */
    void clearMessage();

    /**
     * @brief signal if OpenCL is being used
     * @param useOpenCL true if using OpenCL
     */
    void isUsingOpenCL(bool useOpenCL);

    /**
     * @brief signal if Coulomb is being used
     * @param useCoulomb true if using Coulomb
     */
    void isUsingCoulomb(bool useCoulomb);

    /**
     * @brief signal if simulation is playing
     * @param playing true if playing
     */
    void isAnimated(bool playing);

    /**
     * @brief signal if traps are shown
     * @param shown true if showing traps
     */
    void isShowingTraps(bool shown);

    /**
     * @brief signal that iterations.print changed
     * @param value value of iterations.print
     */
    void iterationsPrintChanged(int value);

    /**
     * @brief signal that current.step changed
     * @param value value of current.step
     */
    void currentStepChanged(int value);

    /**
     * @brief signal that OpenGL has been initialized
     */
    void openGLInitFinished();

    /**
     * @brief signal that the trap color changed
     */
    void trapColorChanged(QColor color);

    /**
     * @brief signal that the background color changed
     */
    void backgroundColorChanged(QColor color);

public slots:
    /**
     * @brief set the render mode for points
     */
    void setElectronPointMode(PointCloud::Mode mode);

    /**
     * @brief set the render mode for points
     */
    void setDefectPointMode(PointCloud::Mode mode);

    /**
     * @brief set the render mode for points
     */
    void setHolePointMode(PointCloud::Mode mode);

    /**
     * @brief set the render mode for points
     */
    void setPointMode(PointCloud::Mode mode);

    /**
     * @brief set the value of iterations.print
     * @param value value to set
     */
    void setIterationsPrint(int value);

    /**
     * @brief turn on the traps texture
     * @param on true is showing traps
     */
    void toggleTrapsShown(bool on=true);

    /**
     * @brief turn OpenCL on and off
     * @param on true if turning OpenCL on
     */
    void toggleOpenCL(bool on=true);

    /**
     * @brief turn Coulomb on and off
     * @param on true if turning Coulomb on
     */
    void toggleCoulomb(bool on=true);

    /**
     * @brief show/hide the corner axis
     */
    void toggleCornerAxisIsVisible();

    /**
     * @brief show/hide the grid
     */
    void toggleGridIsVisible();

    /**
     * @brief load a simulation
     * @param fileName name of simulation input file
     */
    void load(QString fileName);

    /**
     * @brief save a checkpoint file
     * @param fileName name of simulation checkpoint file
     */
    void save(QString fileName);

    /**
     * @brief reset the camera to the default position
     */
    void resetCamera();

    /**
     * @brief unload the current simulation
     */
    void unload();

    /**
     * @brief reset the current simulation
     */
    void reset();

    /**
     * @brief pause the simulation
     */
    void pause();

    /**
     * @brief play the simulation
     */
    void play();

    /**
     * @brief show parameters in a window
     */
    void showParameters();

    /**
     * @brief draw an OpenGL light source
     * @param light
     * @param scale
     */
    void drawLightSource(GLenum light, float scale = 1.0f) const;

    /**
     * @brief load settings from a file
     * @param fileName name of settings file
     */
    void loadSettings(QString fileName);

    /**
     * @brief save settings to a file
     * @param fileName name of settings file
     */
    void saveSettings(QString fileName);

    /**
     * @brief show error message window
     * @param message error message
     */
    void errorMessage(QString message);

    /**
     * @brief set the properties of settings object
     * @param settings settings object
     */
    void setSettings(QSettings &settings);

    /**
     * @brief get the properties of settings object
     * @param settings settings object
     */
    void getSettings(QSettings &settings);

    /**
     * @brief Change the trap color
     * @param color color to set
     */
    void setTrapColor(QColor color);

    /**
     * @brief set background color
     * @param color color to set
     */
    void setBackgroundColor(QColor color);

    /**
     * @brief init trap geometry
     */
    void initTraps();

    /**
     * @brief set default colors
     */
    void resetSettings();

protected:
    /**
     * @brief update the electron point cloud
     */
    void updateElectronCloud();

    /**
     * @brief update the defect point cloud
     */
    void updateDefectCloud();

    /**
     * @brief update the hole point cloud
     */
    void updateHoleCloud();

    /**
     * @brief update the geometry using simulation parameters
     */
    void initGeometry();

    /**
     * @brief setup OpenGL
     */
    virtual void init();

    /**
     * @brief draw on the OpenGL widget before the main draw event
     */
    virtual void preDraw();

    /**
     * @brief draw on the OpenGL widget
     */
    virtual void draw();

    /**
     * @brief draw on the OpenGL widget after the main draw event
     */
    virtual void postDraw();

    /**
     * @brief change the state of the system before drawing
     */
    virtual void animate();

    /**
     * @brief open the help widget
     */
    virtual void help();

    /**
     * @brief get the help string
     */
    virtual QString helpString() const;

    /**
     * @brief draw traps on image
     * @param image image to draw draws on
     */
    void drawTraps(QImage &image, QColor bcolor, QColor fcolor);

    //! axis that sits in the corner and doesnt change size
    CornerAxis *m_cornerAxis;

    //! point cloud representing electrons
    PointCloud *m_electrons;

    //! point cloud representing defects
    PointCloud *m_defects;

    //! point cloud representing holes
    PointCloud *m_holes;

    //! trap box
    Box *m_trapBox;

    //! base box
    Box *m_baseBox;

    //! box (left)
    Box *m_lBox;

    //! box (right)
    Box *m_rBox;

    //! box parameter
    double m_boxThickness;

    //! box parameter
    QColor m_trapColor;

    //! grid that outlines sites
    Grid *m_grid;

    //! trap mesh
    Mesh *m_trapMesh;

    //! main light source
    Light *m_light0;

    //! the simulation manipulator
    Langmuir::Simulation *m_simulation;

    //! a random number generator instance
    Langmuir::Random m_random;

    //! the simulation data
    Langmuir::World *m_world;

    //! half of grid.x
    float m_gridHalfX;

    //! half of grid.y
    float m_gridHalfY;

    //! half of grid.z
    float m_gridHalfZ;

    //! grid.x
    float m_gridX;

    //! grid.y
    float m_gridY;

    //! grid.z
    float m_gridZ;

    //! error messages
    QErrorMessage *m_error;
};

#endif // LANGMUIRVIEWER_H
