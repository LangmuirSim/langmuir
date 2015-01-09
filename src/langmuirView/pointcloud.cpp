#include "langmuirviewer.h"
#include "pointcloud.h"
#include "color.h"
#include "rand.h"

#include <QMetaEnum>

namespace LangmuirView {

PointCloud::PointCloud(LangmuirViewer &viewer, QObject *parent) :
    SceneObject(viewer, parent), m_verticesVBO(NULL)
{
    m_shader1OK = false;
    m_shader2OK = false;
    m_shader3OK = false;
    init();
}

PointCloud::~PointCloud()
{
    if (m_verticesVBO != NULL) {
        delete m_verticesVBO;
    }
}

const QColor& PointCloud::getColor() const
{
    return m_color;
}

float PointCloud::getPointSize() const
{
    return m_pointSize;
}

PointCloud::Mode PointCloud::getMode() const
{
    return m_mode;
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
    m_shader1OK = true;

    // point cloud shader
    if (!m_shader1.addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/pcVert.glsl")) {
        qDebug("langmuir: (pointcloud) can not compile shader1 vertex shader");
        m_shader1OK = false;
    }

    if (!m_shader1.addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/pcFrag.glsl")) {
        qDebug("langmuir: (pointcloud) can not compile shader1 fragment shader");
        m_shader1OK = false;
    }

    if (!m_shader1.link()) {
        qDebug("langmuir: (pointcloud) can not link shader1");
        m_shader1OK = false;
    }

    m_shader2OK = true;

    // square cloud shader
    if (!m_shader2.addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/scVert.glsl")) {
        qDebug("langmuir: (pointcloud) can not compile shader2 vertex shader");
        m_shader2OK = false;
    }

    if (!m_shader2.addShaderFromSourceFile(QOpenGLShader::Geometry, ":shaders/scGeom.glsl")) {
        qDebug("langmuir: (pointcloud) can not compile shader2 geometry shader");
        m_shader2OK = false;
    }

    if (!m_shader2.addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/scFrag.glsl")) {
        qDebug("langmuir: (pointcloud) can not compile shader2 fragment shader");
        m_shader2OK = false;
    }

    if (!m_shader2.link()) {
        qDebug("langmuir: (pointcloud) can not link shader2");
        m_shader2OK = false;
    }

    m_shader3OK = true;

    // square cloud shader
    if (!m_shader3.addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/ccVert.glsl")) {
        qDebug("langmuir: (pointcloud) can not compile shader3 vertex shader");
        m_shader3OK = false;
    }

    if (!m_shader3.addShaderFromSourceFile(QOpenGLShader::Geometry, ":shaders/ccGeom.glsl")) {
        qDebug("langmuir: (pointcloud) can not compile shader3 geometry shader");
        m_shader3OK = false;
    }

    if (!m_shader3.addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/ccFrag.glsl")) {
        qDebug("langmuir: (pointcloud) can not compile shader3 fragment shader");
        m_shader3OK = false;
    }

    if (!m_shader3.link()) {
        qDebug("langmuir: (pointcloud) can not link shader3");
        m_shader3OK = false;
    }

    if (!m_shader1OK) {
        qDebug("langmuir: (pointcloud) shader1 disabled!");
    }

    if (!m_shader2OK) {
        qDebug("langmuir: (pointcloud) shader2 disabled!");
    }

    if (!m_shader3OK) {
        qDebug("langmuir: (pointcloud) shader3 disabled!");
    }
}

QString PointCloud::modeToQString(Mode mode)
{
    QMetaEnum metaEnum = PointCloud::staticMetaObject.enumerator(0);
    return QString(staticMetaObject.enumerator(0).valueToKey(mode));
}

PointCloud::Mode PointCloud::QStringToMode(QString string)
{
    bool ok = false;
    QMetaEnum metaEnum = PointCloud::staticMetaObject.enumerator(0);
    Mode mode = Mode(metaEnum.keysToValue(string.toLatin1(), &ok));
    if (!ok) {
        qFatal("langmuir: can not convert string %s to PointCloud::Mode", qPrintable(string));
    }
    return mode;
}

void PointCloud::init() {

    m_color = Qt::green;
    emit colorChanged(m_color);

    m_mode = Squares;
    emit modeChanged(m_mode);

    m_pointSize = 230.0;
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
            drawPoints();
            break;
        }
        case Squares:
        {
            drawSquares();
            break;
        }
        case Cubes:
        {
            drawCubes();
            break;
        }
        default:
        {
            qFatal("langmuir: implement default!");
            break;
        }
    }
}

void PointCloud::drawFallback()
{
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SMOOTH);

    // shader: pass color
    glColor3f(m_color.redF(), m_color.greenF(), m_color.blueF());

    // shader: pass pointsize
    glPointSize(m_pointSize/32.0);

    // shader: pass vertices
    m_verticesVBO->bind();
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    // shader: draw
    glDrawArrays(GL_POINTS, 0, m_maxRender);

    // release
    m_verticesVBO->release();
    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_PROGRAM_POINT_SIZE);
}

void PointCloud::drawPoints()
{
    if (!m_shader1OK) {
        drawFallback();
        return;
    }

    glEnable(GL_PROGRAM_POINT_SIZE);

    m_shader1.bind();

    // shader: pass matrix
    QMatrix4x4& MV = m_viewer.getOpenGLModelViewMatrix();
    QMatrix4x4& P = m_viewer.getOpenGLProjectionMatrix();
    m_shader1.setUniformValue("matrix", P * MV);

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

    glDisable(GL_PROGRAM_POINT_SIZE);
}

void PointCloud::drawSquares()
{
    if (!m_shader2OK) {
        drawFallback();
        return;
    }

    glEnable(GL_PROGRAM_POINT_SIZE);

    m_shader2.bind();

    // shader: pass matrix
    QMatrix4x4& MV = m_viewer.getOpenGLModelViewMatrix();
    QMatrix4x4& P = m_viewer.getOpenGLProjectionMatrix();
    m_shader2.setUniformValue("matrix", P * MV);

    // shader: pass color
    m_shader2.setUniformValue("color", m_color);

    // shader: pass pointsize
    m_shader2.setUniformValue("pointSize", m_pointSize);

    // shader: pass vertices
    m_verticesVBO->bind();
    m_shader2.enableAttributeArray("vertex");
    m_shader2.setAttributeBuffer("vertex", GL_FLOAT, 0, 3, 0);

    // shader: draw
    glDrawArrays(GL_POINTS, 0, m_maxRender);

    // release
    m_shader2.disableAttributeArray("vertex");
    m_verticesVBO->release();
    m_shader2.release();

    glDisable(GL_PROGRAM_POINT_SIZE);
}

void PointCloud::drawCubes()
{
    if (!m_shader3OK) {
        drawFallback();
        return;
    }

    glEnable(GL_PROGRAM_POINT_SIZE);

    m_shader3.bind();

    // shader: pass matrix
    QMatrix4x4& MV = m_viewer.getOpenGLModelViewMatrix();
    QMatrix4x4& P = m_viewer.getOpenGLProjectionMatrix();
    m_shader3.setUniformValue("matrix", P * MV);

    // shader: pass color
    m_shader3.setUniformValue("color", m_color);

    // shader: pass pointsize
    m_shader3.setUniformValue("pointSize", m_pointSize);

    // shader: pass vertices
    m_verticesVBO->bind();
    m_shader3.enableAttributeArray("vertex");
    m_shader3.setAttributeBuffer("vertex", GL_FLOAT, 0, 3, 0);

    // shader: draw
    glDrawArrays(GL_POINTS, 0, m_maxRender);

    // release
    m_shader3.disableAttributeArray("vertex");
    m_verticesVBO->release();
    m_shader3.release();

    glDisable(GL_PROGRAM_POINT_SIZE);
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
    connect(this, SIGNAL(modeChanged(PointCloud::Mode)), &m_viewer, SLOT(updateGL()));
}

}
