#ifndef LANGMUIRVIEWER_H
#define LANGMUIRVIEWER_H

#include <QGLViewer/qglviewer.h>
#include <QMatrix4x4>

#include "corneraxis.h"
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
    void isAnimated(bool playing);
    void clearMessage();

public slots:
    void toggleCornerAxisIsVisible();
    void load(QString fileName);
    void save(QString fileName);
    void resetCamera();
    void unload();
    void pause();
    void play();

protected:
    virtual void init();
    virtual void draw();
    virtual void postDraw();
    virtual void animate();
    virtual void help();
    virtual QString helpString() const;
    CornerAxis *m_cornerAxis;
    Langmuir::Simulation *m_simulation;
    Langmuir::Random m_random;
    Langmuir::World *m_world;
    QMatrix4x4 m_matrix;
};

#endif // LANGMUIRVIEWER_H
