#ifndef LANGMUIRVIEWER_H
#define LANGMUIRVIEWER_H

#include <QGLViewer/qglviewer.h>
#include "corneraxis.h"

namespace Langmuir {
    class Simulation;
    class World;
}

class LangmuirViewer : public QGLViewer
{
    Q_OBJECT
public:
    explicit LangmuirViewer(QWidget *parent = 0);

public slots:
    void toggleCornerAxisIsVisible();
    void open();

protected:
    virtual void init();
    virtual void draw();
    virtual void postDraw();
    virtual void animate();
    virtual QString helpString() const;
    CornerAxis *m_cornerAxis;
    Langmuir::Simulation *m_simulation;
    Langmuir::World *m_world;
};

#endif // LANGMUIRVIEWER_H
