#include "langmuirviewer.h"
#include "color.h"
#include "box.h"

#include <QVector>
#include <QDebug>
#include <QImage>

namespace LangmuirView {

Box::Box(LangmuirViewer &viewer, QObject *parent) :
    SceneObject(viewer, parent)
{
    m_imageID = 0;
    m_numVertices = 0;
    m_numIndices = 0;
    init();
}

Box::~Box()
{
}

const QColor& Box::getColor() const
{
    return m_color;
}

int Box::getXSize() const
{
    return m_xsize;
}

int Box::getYSize() const
{
    return m_ysize;
}

int Box::getZSize() const
{
    return m_zsize;
}

bool Box::imageIsOn() const
{
    return m_imageOn;
}

void Box::init() {
    m_color     = Qt::red;
    m_xsize     = 1.0;
    m_ysize     = 1.0;
    m_zsize     = 1.0;
    m_halfXSize = 0.5;
    m_halfYSize = 0.5;
    m_halfZSize = 0.5;
    m_imageID   = 0;
    m_imageOn   = false;
    m_faces     = All;

    m_verticesVBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_verticesVBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_verticesVBO->create();

    m_normalsVBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_normalsVBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_normalsVBO->create();

    m_texturesVBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_texturesVBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_texturesVBO->create();

    m_indexVBO = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    m_indexVBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_indexVBO->create();

    emit colorChanged(m_color);
}

void Box::buildGeometry(unsigned int tesselate_x, unsigned int tesselate_y, unsigned int tesselate_z)
{
    float dx = m_xsize / float(tesselate_x);
    float dy = m_ysize / float(tesselate_y);
    float dz = m_zsize / float(tesselate_z);

    QVector<QVector3D> vvectors;
    QVector<QVector3D> nvectors;
    QVector<QVector2D> tvectors;

    QVector<unsigned int> ibuffer;
    QVector<float> vbuffer;
    QVector<float> nbuffer;
    QVector<float> tbuffer;

    // vertex
    float vx = 0.0;
    float vy = 0.0;
    float vz = 0.0;

    // normal
    float nx = 0.0;
    float ny = 0.0;
    float nz = 0.0;

    // texture
    float rx = 1.0 / tesselate_x;
    float ry = 1.0 / tesselate_y;

    float tx = 0.0;
    float ty = 0.0;

    // index
    int index = 0;

    // back face (-Z)
    if (m_faces.testFlag(Back)) {
        vz =-m_halfZSize;
        nx = 0.0;
        ny = 0.0;
        nz =-1.0;
        for (int i = 0; i < tesselate_x; i++) {
            for (int j = 0; j < tesselate_y; j++) {

                vx = i * dx - m_halfXSize;
                vy = j * dy - m_halfYSize ;
                QVector3D v1 = QVector3D(vx     , vy     , vz);
                QVector3D v2 = QVector3D(vx     , vy + dy, vz);
                QVector3D v3 = QVector3D(vx + dx, vy + dy, vz);
                QVector3D v4 = QVector3D(vx + dx, vy     , vz);

                tx = i * rx;
                ty = j * ry;
                QVector2D t1 = QVector2D(tx     , ty     );
                QVector2D t2 = QVector2D(tx     , ty + ry);
                QVector2D t3 = QVector2D(tx + rx, ty + ry);
                QVector2D t4 = QVector2D(tx + rx, ty     );

                QVector3D n = QVector3D(nx, ny, nz);

                index = vvectors.indexOf(v1);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v1);
                    tvectors.push_back(t1);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }

                index = vvectors.indexOf(v2);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v2);
                    tvectors.push_back(t2);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }

                index = vvectors.indexOf(v3);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v3);
                    tvectors.push_back(t3);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }

                index = vvectors.indexOf(v4);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v4);
                    tvectors.push_back(t4);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }
            }
        }
    }

    // front face (+Z)
    if (m_faces.testFlag(Front)) {
        vz = m_halfZSize;
        nx = 0.0;
        ny = 0.0;
        nz = 1.0;
        for (int i = 0; i < tesselate_x; i++) {
            for (int j = 0; j < tesselate_y; j++) {

                vx = i * dx - m_halfXSize;
                vy = j * dy - m_halfYSize ;
                QVector3D v1 = QVector3D(vx     , vy     , vz);
                QVector3D v2 = QVector3D(vx     , vy + dy, vz);
                QVector3D v3 = QVector3D(vx + dx, vy + dy, vz);
                QVector3D v4 = QVector3D(vx + dx, vy     , vz);

                tx = i * rx;
                ty = j * ry;
                QVector2D t1 = QVector2D(tx     , ty     );
                QVector2D t2 = QVector2D(tx     , ty + ry);
                QVector2D t3 = QVector2D(tx + rx, ty + ry);
                QVector2D t4 = QVector2D(tx + rx, ty     );

                QVector3D n = QVector3D(nx, ny, nz);

                index = vvectors.indexOf(v1);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v1);
                    tvectors.push_back(t1);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }

                index = vvectors.indexOf(v2);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v2);
                    tvectors.push_back(t2);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }

                index = vvectors.indexOf(v3);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v3);
                    tvectors.push_back(t3);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }

                index = vvectors.indexOf(v4);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v4);
                    tvectors.push_back(t4);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }
            }
        }
    }

    // north face (+Y)
    if (m_faces.testFlag(North)) {
        vy = m_halfYSize;
        nx = 0.0;
        ny = 1.0;
        nz = 0.0;
        for (int i = 0; i < tesselate_x; i++) {
            for (int j = 0; j < tesselate_z; j++) {

                vx = i * dx - m_halfXSize;
                vz = j * dz - m_halfZSize ;
                QVector3D v1 = QVector3D(vx     , vy, vz     );
                QVector3D v2 = QVector3D(vx     , vy, vz + dz);
                QVector3D v3 = QVector3D(vx + dx, vy, vz + dz);
                QVector3D v4 = QVector3D(vx + dx, vy, vz     );

                tx = i * rx;
                ty = j * ry;
                QVector2D t1 = QVector2D(tx     , ty     );
                QVector2D t2 = QVector2D(tx     , ty + ry);
                QVector2D t3 = QVector2D(tx + rx, ty + ry);
                QVector2D t4 = QVector2D(tx + rx, ty     );

                QVector3D n = QVector3D(nx, ny, nz);

                index = vvectors.indexOf(v1);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v1);
                    tvectors.push_back(t1);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }

                index = vvectors.indexOf(v2);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v2);
                    tvectors.push_back(t2);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }

                index = vvectors.indexOf(v3);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v3);
                    tvectors.push_back(t3);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }

                index = vvectors.indexOf(v4);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v4);
                    tvectors.push_back(t4);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }
            }
        }
    }

    // south face (-Y)
    if (m_faces.testFlag(South)) {
        vy =-m_halfYSize;
        nx = 0.0;
        ny =-1.0;
        nz = 0.0;
        for (int i = 0; i < tesselate_x; i++) {
            for (int j = 0; j < tesselate_z; j++) {

                vx = i * dx - m_halfXSize;
                vz = j * dz - m_halfZSize ;
                QVector3D v1 = QVector3D(vx     , vy, vz     );
                QVector3D v2 = QVector3D(vx     , vy, vz + dz);
                QVector3D v3 = QVector3D(vx + dx, vy, vz + dz);
                QVector3D v4 = QVector3D(vx + dx, vy, vz     );

                tx = i * rx;
                ty = j * ry;
                QVector2D t1 = QVector2D(tx     , ty     );
                QVector2D t2 = QVector2D(tx     , ty + ry);
                QVector2D t3 = QVector2D(tx + rx, ty + ry);
                QVector2D t4 = QVector2D(tx + rx, ty     );

                QVector3D n = QVector3D(nx, ny, nz);

                index = vvectors.indexOf(v1);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v1);
                    tvectors.push_back(t1);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }

                index = vvectors.indexOf(v2);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v2);
                    tvectors.push_back(t2);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }

                index = vvectors.indexOf(v3);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v3);
                    tvectors.push_back(t3);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }

                index = vvectors.indexOf(v4);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v4);
                    tvectors.push_back(t4);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }
            }
        }
    }

    // east face (+Y)
    if (m_faces.testFlag(East)) {
        vx = m_halfXSize;
        nx = 1.0;
        ny = 0.0;
        nz = 0.0;
        for (int i = 0; i < tesselate_y; i++) {
            for (int j = 0; j < tesselate_z; j++) {

                vy = i * dy - m_halfYSize;
                vz = j * dz - m_halfZSize ;
                QVector3D v1 = QVector3D(vx, vy     , vz     );
                QVector3D v2 = QVector3D(vx, vy     , vz + dz);
                QVector3D v3 = QVector3D(vx, vy + dy, vz + dz);
                QVector3D v4 = QVector3D(vx, vy + dy, vz     );

                tx = i * rx;
                ty = j * ry;
                QVector2D t1 = QVector2D(tx     , ty     );
                QVector2D t2 = QVector2D(tx     , ty + ry);
                QVector2D t3 = QVector2D(tx + rx, ty + ry);
                QVector2D t4 = QVector2D(tx + rx, ty     );

                QVector3D n = QVector3D(nx, ny, nz);

                index = vvectors.indexOf(v1);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v1);
                    tvectors.push_back(t1);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }

                index = vvectors.indexOf(v2);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v2);
                    tvectors.push_back(t2);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }

                index = vvectors.indexOf(v3);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v3);
                    tvectors.push_back(t3);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }

                index = vvectors.indexOf(v4);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v4);
                    tvectors.push_back(t4);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }
            }
        }
    }

    // west face (-Y)
    if (m_faces.testFlag(West)) {
        vx =-m_halfXSize;
        nx =-1.0;
        ny = 0.0;
        nz = 0.0;
        for (int i = 0; i < tesselate_y; i++) {
            for (int j = 0; j < tesselate_z; j++) {

                vy = i * dy - m_halfYSize;
                vz = j * dz - m_halfZSize ;
                QVector3D v1 = QVector3D(vx, vy     , vz     );
                QVector3D v2 = QVector3D(vx, vy     , vz + dz);
                QVector3D v3 = QVector3D(vx, vy + dy, vz + dz);
                QVector3D v4 = QVector3D(vx, vy + dy, vz     );

                tx = i * rx;
                ty = j * ry;
                QVector2D t1 = QVector2D(tx     , ty     );
                QVector2D t2 = QVector2D(tx     , ty + ry);
                QVector2D t3 = QVector2D(tx + rx, ty + ry);
                QVector2D t4 = QVector2D(tx + rx, ty     );

                QVector3D n = QVector3D(nx, ny, nz);

                index = vvectors.indexOf(v1);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v1);
                    tvectors.push_back(t1);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }

                index = vvectors.indexOf(v2);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v2);
                    tvectors.push_back(t2);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }

                index = vvectors.indexOf(v3);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v3);
                    tvectors.push_back(t3);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }

                index = vvectors.indexOf(v4);
                if (index >= 0) {
                    ibuffer.push_back(index);
                } else {
                    vvectors.push_back(v4);
                    tvectors.push_back(t4);
                    nvectors.push_back(n);
                    ibuffer.push_back(vvectors.size() - 1);
                }
            }
        }
    }

    vbuffer.reserve(3 * vvectors.size());
    foreach(QVector3D v, vvectors)
    {
        vbuffer.push_back(v.x());
        vbuffer.push_back(v.y());
        vbuffer.push_back(v.z());
    }

    nbuffer.reserve(3 * nvectors.size());
    foreach(QVector3D n, nvectors)
    {
        nbuffer.push_back(n.x());
        nbuffer.push_back(n.y());
        nbuffer.push_back(n.z());
    }

    tbuffer.reserve(2 * tvectors.size());
    foreach(QVector2D t, tvectors)
    {
        tbuffer.push_back(t.x());
        tbuffer.push_back(t.y());
    }

    m_verticesVBO->bind();
    m_verticesVBO->allocate(vbuffer.data(), sizeof(float) * vbuffer.size());
    m_verticesVBO->release();

    m_normalsVBO->bind();
    m_normalsVBO->allocate(nbuffer.data(), sizeof(float) * nbuffer.size());
    m_normalsVBO->release();

    m_texturesVBO->bind();
    m_texturesVBO->allocate(tbuffer.data(), sizeof(float) * tbuffer.size());
    m_texturesVBO->release();

    m_indexVBO->bind();
    m_indexVBO->allocate(ibuffer.data(), sizeof(unsigned int) * ibuffer.size());
    m_indexVBO->release();

    m_numVertices = vbuffer.size();
    m_numIndices = ibuffer.size();
}

void Box::draw() {

    GLboolean texture2DIsOn;
    glGetBooleanv(GL_TEXTURE_2D, &texture2DIsOn);

    if (m_imageOn && m_imageID > 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_imageID);
        glColor3f(1.0, 1.0, 1.0);
    }
    else {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_imageID);
        glDisable(GL_TEXTURE_2D);
        glColor3f(m_color.redF(), m_color.greenF(), m_color.blueF());
    }

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    m_verticesVBO->bind();
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    m_normalsVBO->bind();
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, 0);

    m_texturesVBO->bind();
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);

    m_indexVBO->bind();
    glDrawElements(GL_QUADS, m_numIndices, GL_UNSIGNED_INT, 0);
    m_indexVBO->release();

    m_texturesVBO->release();
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    m_normalsVBO->release();
    glDisableClientState(GL_NORMAL_ARRAY);

    m_verticesVBO->release();
    glDisableClientState(GL_VERTEX_ARRAY);

    if (texture2DIsOn) {
        glEnable(GL_TEXTURE_2D);
    }
}

void Box::setColor(QColor color)
{
    if (color != m_color) {
        m_color = color;
        emit colorChanged(m_color);
    }
}

void Box::setSize(double xvalue, double yvalue, double zvalue, unsigned int tesselate_x,
             unsigned int tesselate_y, unsigned int tesselate_z)
{
    if (tesselate_x <= 0) { tesselate_x = 1; }
    if (tesselate_y <= 0) { tesselate_y = 1; }
    if (tesselate_z <= 0) { tesselate_z = 1; }

    m_xsize = xvalue;
    m_ysize = yvalue;
    m_zsize = zvalue;

    m_halfXSize = 0.5 * m_xsize;
    m_halfYSize = 0.5 * m_ysize;
    m_halfZSize = 0.5 * m_zsize;

    buildGeometry(tesselate_x, tesselate_y, tesselate_z);

    emit sizeChanged(m_xsize, m_ysize, m_zsize);
}

void Box::makeConnections()
{
    SceneObject::makeConnections();
    connect(this, SIGNAL(colorChanged(QColor)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(sizeChanged(double,double,double)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(imageOnChanged(bool)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(facesChanged(Faces)) , &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(textureChanged(GLuint)), &m_viewer, SLOT(updateGL()));
}

void Box::setFaces(Faces faces)
{
    m_faces = faces;
}

void Box::showImage(bool on)
{
    m_imageOn = on;
    emit imageOnChanged(m_imageOn);
}

void Box::toggleImage()
{
    m_imageOn = !m_imageOn;
    emit imageOnChanged(m_imageOn);
}

void Box::setTexture(GLuint imageID)
{
    m_imageID = imageID;
    emit textureChanged(m_imageID);
}

}
