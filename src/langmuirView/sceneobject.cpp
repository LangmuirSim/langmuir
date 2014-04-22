#include "sceneobject.h"

SceneObject::SceneObject(QObject *parent) :
    QObject(parent)
{
}

void SceneObject::render()
{
    preDraw();
    draw();
    postDraw();
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
