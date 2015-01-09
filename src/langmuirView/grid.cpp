#include "langmuirviewer.h"
#include "color.h"
#include "grid.h"

namespace LangmuirView {

Grid::Grid(LangmuirViewer &viewer, QObject *parent) :
    SceneObject(viewer, parent), m_verticesVBO(NULL)
{
    m_shader1OK = false;
    init();
}

Grid::~Grid()
{
    if (m_verticesVBO != NULL) {
        delete m_verticesVBO;
    }
}

const QColor& Grid::getColor() const
{
    return m_color;
}

void Grid::initShaders()
{
    m_shader1OK = true;

    // lines shader
    if (!m_shader1.addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/pcVert.glsl")) {
        qDebug("langmuir: (grid) can not compile shader1 vertex shader");
        m_shader1OK = false;
    }

    if (!m_shader1.addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/pcFrag.glsl")) {
        qDebug("langmuir: (grid) can not compile shader1 fragment shader");
        m_shader1OK = false;
    }

    if (!m_shader1.link()) {
        qDebug("langmuir: (grid) can not link shader1");
        m_shader1OK = false;
    }

    if (!m_shader1OK) {
        qDebug("langmuir: (pointcloud) shader1 disabled!");
        m_shader1OK = false;
    }
}

void Grid::init() {

    m_color = Qt::black;
    emit colorChanged(m_color);

    m_numPoints = 0;

    initShaders();

    m_verticesVBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_verticesVBO->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_verticesVBO->create();
}

void Grid::draw() {

    if (m_numPoints <= 0) {
        return;
    }

    drawFallback();
}

void Grid::drawFallback()
{
    // shader: pass color
    glColor3f(m_color.redF(), m_color.greenF(), m_color.blueF());

    // shader: pass vertices
    m_verticesVBO->bind();
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    // shader: draw
    glDrawArrays(GL_LINES, 0, m_numPoints);

    // release
    m_verticesVBO->release();
    glDisableClientState(GL_VERTEX_ARRAY);
}

void Grid::drawGrid()
{
    if (!m_shader1OK) {
        return;
    }

    m_shader1.bind();

    // shader: pass matrix
    QMatrix4x4& MV = m_viewer.getOpenGLModelViewMatrix();
    QMatrix4x4& P = m_viewer.getOpenGLProjectionMatrix();
    m_shader1.setUniformValue("matrix", P * MV);

    // shader: pass color
    m_shader1.setUniformValue("color", m_color);

    // shader: pass vertices
    m_verticesVBO->bind();
    m_shader1.enableAttributeArray("vertex");
    m_shader1.setAttributeBuffer("vertex", GL_FLOAT, 0, 3, 0);

    // shader: draw
    glDrawArrays(GL_LINES, 0, m_numPoints);

    // release
    m_shader1.disableAttributeArray("vertex");
    m_verticesVBO->release();
    m_shader1.release();
}

void Grid::setDimensions(int xsize, int ysize, int zsize)
{
    QVector<float> vertices;

    m_numPoints = 0;

    for (int i = 0; i < xsize + 1; i++) {
        const float x = i - 0.5 * xsize;
        vertices.push_back(x); vertices.push_back(-0.5 * ysize); vertices.push_back(0.0);
        vertices.push_back(x); vertices.push_back(+0.5 * ysize); vertices.push_back(0.0);
        m_numPoints += 2;
    }

    for (int i = 0; i < ysize + 1; i++) {
        const float y = i - 0.5 * ysize;
        vertices.push_back(-0.5 * xsize); vertices.push_back(y); vertices.push_back(0.0);
        vertices.push_back(+0.5 * xsize); vertices.push_back(y); vertices.push_back(0.0);
        m_numPoints += 2;
    }

    m_verticesVBO->bind();
    m_verticesVBO->allocate(vertices.data(), sizeof(float) * vertices.size());
    m_verticesVBO->release();

    emit gridChanged();
}

void Grid::setColor(QColor color)
{
    if (color != m_color) {
        m_color = color;
        emit colorChanged(m_color);
    }
}

void Grid::makeConnections()
{
    SceneObject::makeConnections();
    connect(this, SIGNAL(colorChanged(QColor)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(gridChanged()), &m_viewer, SLOT(updateGL()));
}

}
