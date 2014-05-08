#include "langmuirviewer.h"
#include "pointcloud.h"
#include "color.h"
#include "rand.h"

PointCloud::PointCloud(LangmuirViewer &viewer, QObject *parent) :
    SceneObject(viewer, parent), m_verticesVBO(NULL)
{
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

    if (!m_shader1.addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/pcFrag.glsl")) {
        qFatal("langmuir: can not compile shader1 fragment shader");
    }

    if (!m_shader1.link()) {
        qFatal("langmuir: can not link shader1");
    }

    // cube cloud shader
    if (!m_shader2.addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/ccVert.glsl")) {
        qFatal("langmuir: can not compile shader2 vertex shader");
    }

    if (!m_shader2.addShaderFromSourceFile(QOpenGLShader::Geometry, ":shaders/ccGeom.glsl")) {
        qFatal("langmuir: can not compile shader2 geometry shader");
    }

    if (!m_shader2.addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/ccFrag.glsl")) {
        qFatal("langmuir: can not compile shader2 fragment shader");
    }

    if (!m_shader2.link()) {
        qFatal("langmuir: can not link shader2");
    }
}

void PointCloud::init() {

    m_color = Qt::green;
    emit colorChanged(m_color);

    m_mode = Points;
    emit modeChanged(m_mode);

    m_pointSize = 5.0;
    emit pointSizeChanged(m_pointSize);

    m_maxPoints = 0;
    emit maxPointsChanged(m_maxPoints);

    m_maxRender = 0;
    emit maxRenderChanged(m_maxRender);

    initShaders();

    m_vertices.clear();
    m_vertices.resize(3 * m_maxPoints);

    m_verticesVBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_verticesVBO->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_verticesVBO->create();
    m_verticesVBO->bind();
    m_verticesVBO->allocate(m_vertices.data(), sizeof(float) * m_vertices.size());
    m_verticesVBO->release();
}

void PointCloud::draw() {

    if (m_maxRender == 0) {
        return;
    }

    switch(m_mode)
    {
        case Points:
        {
            glEnable(GL_PROGRAM_POINT_SIZE);

            m_shader1.bind();

            // shader: pass matrix
            QMatrix4x4& MVP = m_viewer.getMVP();
            m_shader1.setUniformValue("matrix", MVP);

            // shader: pass color
            m_shader1.setUniformValue("color", m_color);

            // shader: pass pointsize
            m_shader1.setUniformValue("pointSize", m_pointSize);

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

            //glDisable(GL_PROGRAM_POINT_SIZE);

            break;
        }
        case Cubes:
        {
            break;
        }
        default:
        {
            qFatal("langmuir: implement default!");
            break;
        }
    }
}

void PointCloud::setMaxPoints(unsigned int value)
{
    if (value != m_maxPoints) {
        m_maxPoints = value;
        m_maxRender = 0;

        m_vertices.clear();
        m_vertices.resize(3 * m_maxPoints);

        m_verticesVBO->bind();
        m_verticesVBO->allocate(sizeof(float) * 3 * m_maxPoints);
        m_verticesVBO->write(0, m_vertices.data(), sizeof(float) * 3 * m_maxPoints);
        m_verticesVBO->release();

        emit maxPointsChanged(m_maxPoints);
        emit maxRenderChanged(m_maxRender);
    }
}

void PointCloud::setMaxRender(unsigned int value)
{
    if (value != m_maxRender) {
        if (value > m_maxPoints) {
            qFatal("langmuir: VBO must be resized");
        }
        m_maxRender = value;
        emit maxRenderChanged(m_maxRender);
    }
}

void PointCloud::setPointSize(float value)
{
    if (value != m_pointSize) {
        m_pointSize = value;
        emit pointSizeChanged(m_pointSize);
    }
}

void PointCloud::setColor(QColor color)
{
    if (color != m_color) {
        m_color = color;
        emit colorChanged(m_color);
    }
}

void PointCloud::setMode(Mode mode)
{
    if (mode != m_mode) {
        m_mode = mode;
        emit modeChanged(m_mode);
    }
}

void PointCloud::updateVBO()
{
    if (m_vertices.size() != 3 * m_maxPoints) {
        qFatal("langmuir: VBO(%d) != vertices(%d)", 3 * m_maxPoints, m_vertices.size());
    }

    if (m_maxRender > m_maxPoints) {
        qFatal("langmuir: maxRender(%d) > maxPoints(%d)", m_maxRender, m_maxPoints);
    }

    m_verticesVBO->bind();
    m_verticesVBO->write(0, m_vertices.data(), sizeof(float) * 3 * m_maxRender);
    m_verticesVBO->release();
}

void PointCloud::makeConnections()
{
    SceneObject::makeConnections();
    connect(this, SIGNAL(pointSizeChanged(float)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(colorChanged(QColor)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(modeChanged(Mode)), &m_viewer, SLOT(updateGL()));
}
