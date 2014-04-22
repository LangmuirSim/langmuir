#include "sceneobject.h"
#include "langmuirviewer.h"
#include <QDebug>

SceneObject::SceneObject(LangmuirViewer &viewer, QObject *parent) :
    QObject(parent), m_viewer(viewer), init_(false)
{
    init();
}

void SceneObject::qColorToFloat3(QColor qcolor, float *color)
{
    color[0] = qcolor.redF();
    color[1] = qcolor.greenF();
    color[2] = qcolor.blueF();
}

void SceneObject::qColorToFloat4(QColor qcolor, float *color)
{
    qColorToFloat3(qcolor, color);
    color[3] = qcolor.alphaF();
}

void SceneObject::render()
{
    preDraw();
    draw();
    postDraw();
}

void SceneObject::init()
{
}

void SceneObject::draw()
{
}

void SceneObject::preDraw()
{
}

void SceneObject::postDraw()
{
}

bool SceneObject::isVisible()
{
    return visible_;
}

void SceneObject::toggleVisible()
{
    visible_ = ! visible_;
    emit visibleChanged(visible_);
}

void SceneObject::setVisible(bool draw)
{
    visible_ = draw;
    emit visibleChanged(visible_);
}
