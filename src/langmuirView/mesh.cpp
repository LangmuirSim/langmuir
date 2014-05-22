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

Mesh::Mode Mesh::getMode() const
{
    return m_mode;
}

QString Mesh::modeToQString(Mesh::Mode mode)
{
    QMetaEnum metaEnum = Mesh::staticMetaObject.enumerator(0);
    return QString(metaEnum.valueToKey(mode));
}

Mesh::Mode Mesh::QStringToMode(QString string)
{
    bool ok = false;
    QMetaEnum metaEnum = Mesh::staticMetaObject.enumerator(0);
    Mode mode = Mode(metaEnum.keysToValue(string.toLatin1(), &ok));
    if (!ok) {
        qFatal("langmuir: can not convert string %s to Mesh::Mode", qPrintable(string));
    }
    return mode;
}

void Mesh::init() {

    m_colorA = Qt::red;
    emit colorAChanged(m_colorA);

    m_colorB = Qt::yellow;
    emit colorBChanged(m_colorB);

    m_mode = DoubleAlpha;
    emit modeChanged(m_mode);

    initShaders();

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

    switch (m_mode)
    {
        case Single: default:
        {
            drawSingle();
            break;
        }
        case Double:
        {
            drawDouble();
            break;
        }
        case SingleAlpha:
        {
            drawSingleAlpha();
            break;
        }
        case DoubleAlpha:
        {
            drawDoubleAlpha();
            break;
        }
        case Shader1:
        {
            drawShader1();
            break;
        }
    }
}

void Mesh::initShaders()
{
    // point cloud shader
    if (!m_shader1.addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/msVert.glsl")) {
        qFatal("langmuir: can not compile shader1 vertex shader");
    }

//    if (!m_shader1.addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":shaders/msTesC.glsl")) {
//        qFatal("langmuir: can not compile shader1 tesselation control shader");
//    }

//    if (!m_shader1.addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":shaders/msTesE.glsl")) {
//        qFatal("langmuir: can not compile shader1 tesselation evaluation shader");
//    }

    if (!m_shader1.addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/msFrag.glsl")) {
        qFatal("langmuir: can not compile shader1 fragment shader");
    }

    if (!m_shader1.link()) {
        qFatal("langmuir: can not link shader1");
    }
}

void Mesh::drawSingle()
{
    static float colorA[4];
    color::qColorToArray4(m_colorA, colorA);

    GLboolean texture2DIsOn;
    glGetBooleanv(GL_TEXTURE_2D, &texture2DIsOn);
    glDisable(GL_TEXTURE_2D);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    //glEnable(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);

    m_verticesVBO->bind();
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    m_normalsVBO->bind();
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, 0);

    m_indexVBO->bind();

    // front face
    //glCullFace(GL_BACK);
    glColor4fv(colorA);
    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    m_indexVBO->release();

    m_normalsVBO->release();
    glDisableClientState(GL_NORMAL_ARRAY);

    m_verticesVBO->release();
    glDisableClientState(GL_VERTEX_ARRAY);

    //glDisable(GL_CULL_FACE);

    if (texture2DIsOn) {
        glEnable(GL_TEXTURE_2D);
    }
}

void Mesh::drawSingleAlpha()
{
    static float colorA[4];
    color::qColorToArray4(m_colorA, colorA);

    GLboolean texture2DIsOn;
    glGetBooleanv(GL_TEXTURE_2D, &texture2DIsOn);
    glDisable(GL_TEXTURE_2D);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    //glEnable(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);

    m_verticesVBO->bind();
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    m_normalsVBO->bind();
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, 0);

    m_indexVBO->bind();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // front face
    //glCullFace(GL_BACK);
    glColor4fv(colorA);
    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    glDisable(GL_BLEND);

    m_indexVBO->release();

    m_normalsVBO->release();
    glDisableClientState(GL_NORMAL_ARRAY);

    m_verticesVBO->release();
    glDisableClientState(GL_VERTEX_ARRAY);

    //glDisable(GL_CULL_FACE);

    if (texture2DIsOn) {
        glEnable(GL_TEXTURE_2D);
    }
}

void Mesh::drawDouble()
{
    static float colorA[4];
    color::qColorToArray4(m_colorA, colorA);

    static float colorB[4];
    color::qColorToArray4(m_colorB, colorB);

    GLboolean texture2DIsOn;
    glGetBooleanv(GL_TEXTURE_2D, &texture2DIsOn);
    glDisable(GL_TEXTURE_2D);

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
    glCullFace(GL_BACK);
    glColor4fv(colorA);
    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    // back face
    glCullFace(GL_FRONT);
    glColor4fv(colorB);
    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    m_indexVBO->release();

    m_normalsVBO->release();
    glDisableClientState(GL_NORMAL_ARRAY);

    m_verticesVBO->release();
    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable(GL_CULL_FACE);

    if (texture2DIsOn) {
        glEnable(GL_TEXTURE_2D);
    }
}

void Mesh::drawDoubleAlpha()
{
    static float colorA[4];
    color::qColorToArray4(m_colorA, colorA);

    static float colorB[4];
    color::qColorToArray4(m_colorB, colorB);

    GLboolean texture2DIsOn;
    glGetBooleanv(GL_TEXTURE_2D, &texture2DIsOn);
    glDisable(GL_TEXTURE_2D);

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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // front face
    glCullFace(GL_BACK);
    glColor4fv(colorA);
    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    // back face
    glCullFace(GL_FRONT);
    glColor4fv(colorB);
    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    glDisable(GL_BLEND);

    m_indexVBO->release();

    m_normalsVBO->release();
    glDisableClientState(GL_NORMAL_ARRAY);

    m_verticesVBO->release();
    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable(GL_CULL_FACE);

    if (texture2DIsOn) {
        glEnable(GL_TEXTURE_2D);
    }
}

void Mesh::drawShader1()
{
    // bind shader
    m_shader1.bind();

    // shader: pass matrix
    QMatrix4x4& MV = m_viewer.getOpenGLModelViewMatrix();
    QMatrix4x4& P = m_viewer.getOpenGLProjectionMatrix();
    m_shader1.setUniformValue("MV", MV);
    m_shader1.setUniformValue("MVP", P * MV);

    // shader: pass light
    m_shader1.setUniformValue("light_colorA", m_viewer.light().getAColor());
    m_shader1.setUniformValue("light_colorS", m_viewer.light().getSColor());
    m_shader1.setUniformValue("light_colorD", m_viewer.light().getDColor());
    m_shader1.setUniformValue("light_vertex", m_viewer.light().getPosition());

    // shader: pass vertices
    m_verticesVBO->bind();
    m_shader1.enableAttributeArray("vertex");
    m_shader1.setAttributeBuffer("vertex", GL_FLOAT, 0, 3, 0);

    // shader: pass normals
    m_normalsVBO->bind();
    m_shader1.enableAttributeArray("normal");
    m_shader1.setAttributeBuffer("normal", GL_FLOAT, 0, 3, 0);

    // shader: pass color
    m_shader1.setUniformValue("colorA", m_colorA);
    m_shader1.setUniformValue("colorB", m_colorB);

    // draw
    //glDrawArrays(GL_TRIANGLES, 0, m_numVertices);

    // bind index
    m_indexVBO->bind();
    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);
    m_indexVBO->release();

    // disable vertex
    m_shader1.disableAttributeArray("vertex");
    m_verticesVBO->release();

    // disable normal
    m_shader1.disableAttributeArray("normal");
    m_normalsVBO->release();

    // release shader
    m_shader1.release();
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

void Mesh::setMode(Mode mode)
{
    if (mode != m_mode) {
        m_mode = mode;
        emit modeChanged(m_mode);
    }
}

void Mesh::clear()
{
    QVector<float> v;
    QVector<float> n;
    QVector<unsigned int> i;
    setMesh(v, n, i);
}

void Mesh::makeConnections()
{
    SceneObject::makeConnections();
    connect(this, SIGNAL(colorAChanged(QColor)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(colorBChanged(QColor)), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(meshChanged()), &m_viewer, SLOT(updateGL()));
    connect(this, SIGNAL(modeChanged(Mesh::Mode)), &m_viewer, SLOT(updateGL()));
}
