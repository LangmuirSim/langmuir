#ifndef LANGMUIRVIEWER_H
#define LANGMUIRVIEWER_H

#include <QGLViewer/qglviewer.h>
#include <QMatrix4x4>

#include "corneraxis.h"
#include "pointcloud.h"
#include "grid.h"
#include "rand.h"

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
     * @brief obtain the model-view-projection matrix
     */
    QMatrix4x4& getMVP();

    /**
     * @brief get the random number generator
     */
    Langmuir::Random& random();

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
     * @brief signal if simulation is playing
     * @param playing true if playing
     */
    void isAnimated(bool playing);

public slots:
    /**
     * @brief turn OpenCL on and off
     * @param on true if turning OpenCL on
     */
    void toggleOpenCL(bool on=true);

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
     * @brief pause the simulation
     */
    void pause();

    /**
     * @brief play the simulation
     */
    void play();

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

    //! axis that sits in the corner and doesnt change size
    CornerAxis *m_cornerAxis;

    //! point cloud representing electrons
    PointCloud *m_electons;

    //! point cloud representing defects
    PointCloud *m_defects;

    //! point cloud representing holes
    PointCloud *m_holes;

    //! grid that outlines sites
    Grid *m_grid;

    //! the simulation manipulator
    Langmuir::Simulation *m_simulation;

    //! a random number generator instance
    Langmuir::Random m_random;

    //! the simulation data
    Langmuir::World *m_world;

    //! storage for the MVP matrix
    QMatrix4x4 m_matrix;

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
};

#endif // LANGMUIRVIEWER_H
