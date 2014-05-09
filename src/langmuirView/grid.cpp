#include "langmuirviewer.h"
#include "color.h"
#include "grid.h"

Grid::Grid(LangmuirViewer &viewer, QObject *parent) :
    SceneObject(viewer, parent), m_verticesVBO(NULL)
{
    init();
}

Grid::~Grid()
{
    if (m_verticesVBO != NULL) {
        delete m_verticesVBO;
    }
}

void Grid::initShaders()
{
    // lines shader
    if (!m_shader1.addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/pcVert.glsl")) {
        qFatal("langmuir: can not compile shader1 vertex shader");
    }

    if (!m_shader1.addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/pcFrag.glsl")) {
        qFatal("langmuir: can not compile shader1 fragment shader");
    }

    if (!m_shader1.link()) {
        qFatal("langmuir: can not link shader1");
    }
}

void Grid::init() {

    m_color = Qt::white;
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

    m_shader1.bind();

    // shader: pass matrix
    QMatrix4x4& MVP = m_viewer.getMVP();

    m_shader1.setUniformValue("matrix", MVP);

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
