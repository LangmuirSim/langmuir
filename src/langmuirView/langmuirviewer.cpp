#include "langmuirviewer.h"
#include "checkpointer.h"
#include "parameters.h"
#include "simulation.h"
#include "world.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QString>
#include <QColor>
#include <QDebug>
#include <QDir>

#include "vec.h"

LangmuirViewer::LangmuirViewer(QWidget *parent) :
    QGLViewer(parent)
{
    m_simulation = NULL;
    m_world = NULL;

    QGLFormat format;
    format.setVersion(4, 0);
    setFormat(format);

    qDebug("langmuir: OpenGL %d.%d", context()->format().majorVersion(), context()->format().minorVersion());
}

QMatrix4x4& LangmuirViewer::getModelViewProjectionMatrix()
{
    static GLdouble matrix[16];
    camera()->getModelViewProjectionMatrix(matrix);

    m_matrix(0, 0) = matrix[0];
    m_matrix(1, 0) = matrix[1];
    m_matrix(2, 0) = matrix[2];
    m_matrix(3, 0) = matrix[3];

    m_matrix(0, 1) = matrix[4];
    m_matrix(1, 1) = matrix[5];
    m_matrix(2, 1) = matrix[6];
    m_matrix(3, 1) = matrix[7];

    m_matrix(0, 2) = matrix[8];
    m_matrix(1, 2) = matrix[9];
    m_matrix(2, 2) = matrix[10];
    m_matrix(3, 2) = matrix[11];

    m_matrix(0, 3) = matrix[12];
    m_matrix(1, 3) = matrix[13];
    m_matrix(2, 3) = matrix[14];
    m_matrix(3, 3) = matrix[15];

    return m_matrix;
}

Langmuir::Random& LangmuirViewer::random()
{
    return m_random;
}

void LangmuirViewer::init()
{
    // Corner axis
    m_cornerAxis = new CornerAxis(*this, this);
    m_cornerAxis->setVisible(false);
    m_cornerAxis->makeConnections();

    // Electrons
    m_electons = new PointCloud(*this, this);
    m_electons->setColor(Qt::red);
    m_electons->setVisible(true);
    m_electons->makeConnections();

    // Defects
    m_defects = new PointCloud(*this, this);
    m_defects->setColor(Qt::white);
    m_defects->setVisible(true);
    m_defects->makeConnections();

    // Holes
    m_holes = new PointCloud(*this, this);
    m_holes->setColor(Qt::blue);
    m_holes->setVisible(true);
    m_holes->makeConnections();

    // Light setup
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    const GLfloat light_p[4] = {1.0, 1.0, 1.0, 0.0};
    const GLfloat light_a[4] = {0.0, 0.0, 0.0, 1.0};
    const GLfloat light_s[4] = {1.0, 1.0, 1.0, 1.0};
    const GLfloat light_d[4] = {1.0, 1.0, 1.0, 1.0};

    glLightfv(GL_LIGHT0, GL_POSITION, light_p);
    glLightfv(GL_LIGHT0, GL_AMBIENT , light_a);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_s);
    glLightfv(GL_LIGHT0, GL_DIFFUSE , light_d);

    // Camera setup
    setSceneRadius(25.0);
    showEntireScene();

    // Background
    setBackgroundColor(Qt::black);

    // OpenGL
    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);
}

void LangmuirViewer::draw()
{
    m_electons->render();
    m_defects->render();
    m_holes->render();
}

void LangmuirViewer::postDraw()
{
    QGLViewer::postDraw();
    m_cornerAxis->render();
}

void LangmuirViewer::animate()
{
    if (m_simulation == NULL || m_world == NULL) {
        pause();
    }
}

void LangmuirViewer::help() {
    emit showMessage("implement help widget!");
}

QString LangmuirViewer::helpString() const
{
    return "";
}

void LangmuirViewer::toggleCornerAxisIsVisible()
{
    m_cornerAxis->toggleVisible();
    emit showMessage(QString("corner axis=%1").arg(m_cornerAxis->isVisible()));
}

void LangmuirViewer::load(QString fileName)
{
    if (animationIsStarted()) {
        pause();
    }

    if (fileName.isEmpty()) {
        emit showMessage("no input file selected");
        return;
    }
    emit showMessage(QString("loading file: %1").arg(fileName));

    Langmuir::World *world = new Langmuir::World(fileName, -1, -1, this);
    Langmuir::Simulation *simulation = new Langmuir::Simulation(*m_world, m_world);

    world->parameters().outputIsOn = false;
    world->parameters().iterationsPrint = 1;

    unload();

    m_simulation = simulation;
    m_world = world;
}

void LangmuirViewer::save(QString fileName)
{
    if (animationIsStarted()) {
        pause();
    }

    if (m_world != NULL || fileName.isEmpty()) {
        m_world->checkPointer().save(fileName);
        emit showMessage(qPrintable(QString("saved: %1").arg(fileName)));
    }
    else {
        QMessageBox::warning(this, "Langmuir", "Can not save simulation");
    }
}

void LangmuirViewer::unload()
{
    if (animationIsStarted()) {
        pause();
    }

    if (m_simulation == NULL || m_world == NULL) {
        emit showMessage("nothing to delete");
    } else {
        emit showMessage("simulation deleted");
    }

    if (m_simulation != NULL) {
        m_simulation->deleteLater();
    }

    if (m_world != NULL) {
        m_world->deleteLater();
    }

    m_simulation = NULL;
    m_world = NULL;
}

void LangmuirViewer::resetCamera()
{
    camera()->setUpVector(qglviewer::Vec(0, 1, 0));
    camera()->setViewDirection(qglviewer::Vec(0, 0, -1));
    camera()->showEntireScene();
    updateGL();
}

void LangmuirViewer::pause()
{
    if (animationIsStarted()) {
        stopAnimation();
        emit showMessage("paused");
    }
    emit isAnimated(animationIsStarted());
}

void LangmuirViewer::play()
{
    if (animationIsStarted()) {
    } else {
        startAnimation();
        emit showMessage("playing");
    }
    emit isAnimated(animationIsStarted());
}
