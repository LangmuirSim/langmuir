#include "langmuirviewer.h"
#include "pointcloud.h"
#include "color.h"

PointCloud::PointCloud(LangmuirViewer &viewer, QObject *parent) :
    SceneObject(viewer, parent)
{
}

void PointCloud::init() {
    m_color = Qt::red;

    emit colorChanged(m_color);
}

void PointCloud::draw() {
}

void PointCloud::setColor(QColor color)
{
    if (color != m_color) {
        m_color = color;
        emit colorChanged(m_color);
    }
}

void PointCloud::makeConnections()
{
    SceneObject::makeConnections();
    connect(this, SIGNAL(colorChanged(QColor)), &m_viewer, SLOT(updateGL()));
}
