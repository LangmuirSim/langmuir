#ifndef LANGMUIRVIEWER_H
#define LANGMUIRVIEWER_H

#include <QGLViewer/qglviewer.h>
#include <QMatrix4x4>

#include "corneraxis.h"
#include "pointcloud.h"
#include "rand.h"

namespace Langmuir {
    class Simulation;
    class World;
}

class LangmuirViewer : public QGLViewer
{
    Q_OBJECT
public:
    explicit LangmuirViewer(QWidget *parent = 0);
    QMatrix4x4& getModelViewProjectionMatrix();
    Langmuir::Random& random();

signals:
    void showMessage(const QString& message, int timeout=0);
    void isUsingOpenCL(bool useOpenCL);
    void isAnimated(bool playing);
    void clearMessage();

public slots:
    void toggleOpenCL(bool on=true);
    void toggleCornerAxisIsVisible();
    void load(QString fileName);
    void save(QString fileName);
    void resetCamera();
    void unload();
    void pause();
    void play();

protected:
    void updateElectronCloud();
    void updateDefectCloud();
    void updateHoleCloud();
    void initGeometry();

    virtual void init();
    virtual void draw();
    virtual void postDraw();
    virtual void animate();
    virtual void help();
    virtual QString helpString() const;

    CornerAxis *m_cornerAxis;
    PointCloud *m_electons;
    PointCloud *m_defects;
    PointCloud *m_holes;

    Langmuir::Simulation *m_simulation;
    Langmuir::Random m_random;
    Langmuir::World *m_world;

    QMatrix4x4 m_matrix;
    float m_gridHalfX;
    float m_gridHalfY;
    float m_gridHalfZ;
    float m_gridX;
    float m_gridY;
    float m_gridZ;
};

#endif // LANGMUIRVIEWER_H
