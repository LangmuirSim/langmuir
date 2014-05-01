#include "sceneobject.h"
#include "langmuirviewer.h"
#include <QDebug>

SceneObject::SceneObject(LangmuirViewer &viewer, QObject *parent) :
    QObject(parent), m_viewer(viewer), visible_(true)
{
    init();
}

void SceneObject::render()
{
    if (visible_) {
        preDraw();
        draw();
        postDraw();
    }
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

void SceneObject::makeConnections()
{
    connect(this, SIGNAL(visibleChanged(bool)), &m_viewer, SLOT(updateGL()));
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
