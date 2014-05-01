#include "langmuirviewer.h"
#include "box.h"
#include <QDebug>

Box::Box(LangmuirViewer &viewer, QObject *parent) :
    SceneObject(viewer, parent)
{
}

void Box::init() {
    m_color = QColor::fromRgbF(1.0, 0.0, 0.0, 0.0);
    emit colorChanged(m_color);
}

void Box::draw() {
}

void Box::setColor(QColor color)
{
    if (color != m_color) {
        m_color = color;
        emit colorChanged(m_color);
    }
}

void Box::makeConnections()
{
    SceneObject::makeConnections();
    connect(this, SIGNAL(colorChanged(QColor)), &m_viewer, SLOT(updateGL()));
}
