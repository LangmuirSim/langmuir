#include "langmuirviewer.h"
#include "pointcloud.h"
#include "color.h"
#include "rand.h"

PointCloud::PointCloud(LangmuirViewer &viewer, QObject *parent) :
    SceneObject(viewer, parent), m_verticesVBO(NULL)
{
    m_maxRender = 256;
    m_maxPoints = 1000;
    init();
}

PointCloud::~PointCloud()
{
    if (m_verticesVBO != NULL) {
        delete m_verticesVBO;
    }
}

QVector<float>& PointCloud::vertices()
{
    return m_vertices;
}

unsigned int PointCloud::getMaxPoints()
{
    return m_maxPoints;
}

unsigned int PointCloud::getMaxRender()
{
    return m_maxRender;
}

void PointCloud::initShaders()
{
    // point cloud shader
    if (!m_shader1.addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/pcVert.glsl")) {
        qFatal("langmuir: can not compile shader1 vertex shader");
    }

    if (!m_shader1.addShaderFromSourceFile(QOpenGLShader::Geometry, ":shaders/pcGeom.glsl")) {
        qFatal("langmuir: can not compile shader1 geometry shader");
    }

    if (!m_shader1.addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/pcFrag.glsl")) {
        qFatal("langmuir: can not compile shader1 fragment shader");
    }

    if (!m_shader1.link()) {
        qFatal("langmuir: can not link shader");
    }

    // cube cloud shader
    if (!m_shader2.addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/ccVert.glsl")) {
        qFatal("langmuir: can not compile shader1 vertex shader");
    }

    if (!m_shader2.addShaderFromSourceFile(QOpenGLShader::Geometry, ":shaders/ccGeom.glsl")) {
        qFatal("langmuir: can not compile shader1 geometry shader");
    }

    if (!m_shader2.addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/ccFrag.glsl")) {
        qFatal("langmuir: can not compile shader1 fragment shader");
    }

    if (!m_shader2.link()) {
        qFatal("langmuir: can not link shader");
    }
}

void PointCloud::init() {
    m_color = Qt::green;

    emit colorChanged(m_color);

    initShaders();

    sleep(1);
    Langmuir::Random rand;
    qDebug() << rand.random();

    for (int i = 0; i < 3 * m_maxPoints; i++) {
        m_vertices.push_back(rand.range(-5, 5));
    }

    m_verticesVBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_verticesVBO->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_verticesVBO->create();
    m_verticesVBO->bind();
    m_verticesVBO->allocate(m_vertices.data(), sizeof(float) * m_vertices.size());
    m_verticesVBO->release();
}

void PointCloud::draw() {
    glEnable(GL_PROGRAM_POINT_SIZE);

    m_shader1.bind();

    // shader: pass matrix
    QMatrix4x4& MVP = m_viewer.getModelViewProjectionMatrix();
    m_shader1.setUniformValue("matrix", MVP);

    // shader: pass color
    m_shader1.setUniformValue("color", m_color);

    // shader: pass vertices
    m_verticesVBO->bind();
    m_shader1.enableAttributeArray("vertex");
    m_shader1.setAttributeBuffer("vertex", GL_FLOAT, 0, 3, 0);

    // shader: draw
    glDrawArrays(GL_POINTS, 0, m_maxRender);

    // release
    m_shader1.disableAttributeArray("vertex");
    m_verticesVBO->release();
    m_shader1.release();

    glDisable(GL_PROGRAM_POINT_SIZE);
}

void PointCloud::setMaxRender(unsigned int value)
{
    if (value != m_maxRender && m_maxRender <= m_maxPoints) {
        m_maxRender = value;
        emit maxRenderChanged(m_maxRender);
    }
}

void PointCloud::setColor(QColor color)
{
    if (color != m_color) {
        m_color = color;
        emit colorChanged(m_color);
    }
}

void PointCloud::updateVBO()
{
    m_verticesVBO->bind();
    m_verticesVBO->write(0, m_vertices.data(), sizeof(float) * m_maxRender);
    m_verticesVBO->release();
    emit vboChanged();
}

void PointCloud::makeConnections()
{
    SceneObject::makeConnections();
    connect(this, SIGNAL(colorChanged(QColor)), &m_viewer, SLOT(updateGL()));
}
