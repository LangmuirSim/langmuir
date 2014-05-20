#include "langmuirviewer.h"
#include "color.h"
#include "mesh.h"
#include "rand.h"

#include <QMetaEnum>

Mesh::Mesh(LangmuirViewer &viewer, QObject *parent) :
    SceneObject(viewer, parent), m_verticesVBO(NULL)
{
    m_numVertices = 0;
    m_numIndices = 0;
    init();
}

Mesh::~Mesh()
{
    if (m_verticesVBO != NULL) {
        delete m_verticesVBO;
    }
}

const QColor& Mesh::getColorA() const
{
    return m_colorA;
}

const QColor& Mesh::getColorB() const
{
    return m_colorB;
}

void Mesh::init() {

    m_colorA = Qt::red;
    emit colorAChanged(m_colorA);

    m_colorB = Qt::yellow;
    emit colorBChanged(m_colorB);

    m_verticesVBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_verticesVBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_verticesVBO->create();

    m_normalsVBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_normalsVBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_normalsVBO->create();

    m_indexVBO = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    m_indexVBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_indexVBO->create();
}

void Mesh::draw() {

    if (m_numVertices <= 0 || m_numIndices <= 0) {
        return;
    }

    static float color[4];

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glEnable(GL_CULL_FACE);

    m_verticesVBO->bind();
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    m_normalsVBO->bind();
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, 0);

    m_indexVBO->bind();

    // front face
    color::qColorToArray4(m_colorA, color);
    glCullFace(GL_BACK);
    glColor4fv(color);
    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    // back face
    color::qColorToArray4(m_colorB, color);
    glCullFace(GL_FRONT);
    glColor4fv(color);
    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    m_indexVBO->release();

    m_normalsVBO->release();
    glDisableClientState(GL_NORMAL_ARRAY);

    m_verticesVBO->release();
    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable(GL_CULL_FACE);
}

void Mesh::setColorA(QColor color)
{
    if (color != m_colorA) {
        m_colorA = color;
        emit colorAChanged(m_colorA);
    }
}

void Mesh::setColorB(QColor color)
{
    if (color != m_colorB) {
        m_colorB = color;
        emit colorBChanged(m_colorB);
    }
}

void Mesh::setMesh(const QVector<float> &vertices, const QVector<float> &normals, const QVector<unsigned int> &indices)
{
    if (vertices.size() != normals.size()) {
        qFatal("langmuir: vertex and normal buffers must be the same size");
    }

    if (indices.size() % 3 != 0) {
        qFatal("langmuir: invalid number of indices in index buffer");
    }

    foreach(GLuint index, indices) {
        if (index < 0 || index >= vertices.size()) {
            qFatal("langmuir: index out of range in index buffer");
        }
    }

    m_verticesVBO->bind();
    m_verticesVBO->allocate(vertices.data(), sizeof(float) * vertices.size());
    m_verticesVBO->release();

    m_normalsVBO->bind();
    m_normalsVBO->allocate(normals.data(), sizeof(float) * normals.size());
    m_normalsVBO->release();

    m_indexVBO->bind();
    m_indexVBO->allocate(indices.data(), sizeof(unsigned int) * indices.size());
    m_indexVBO->release();

    m_numVertices = vertices.size();
    m_numIndices = indices.size();

    emit meshChanged();
}

void Mesh::makeConnections()
{
    SceneObject::makeConnections();
    connect(this, SIGNAL(colorAChanged(QColor)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(colorBChanged(QColor)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(meshChanged()), &m_viewer, SLOT(updateGL()));
}
