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
}

void LangmuirViewer::init()
{
    // Object setup
    m_cornerAxis = new CornerAxis(*this, this);
    m_cornerAxis->setVisible(false);
    m_cornerAxis->makeConnections();

    // Light setup
    glDisable(GL_LIGHT0);

    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT1);

    const GLfloat light_a[4] = {0.5, 0.5, 0.5, 0.5};
    const GLfloat light_s[4] = {0.0, 0.0, 0.0, 0.0};
    const GLfloat light_d[4] = {0.5, 0.5, 0.5, 0.5};

    glLightfv(GL_LIGHT1, GL_AMBIENT , light_a);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_s);
    glLightfv(GL_LIGHT1, GL_DIFFUSE , light_d);

    // Camera setup
    setSceneRadius(5.0);
    showEntireScene();

    // Background
    setBackgroundColor(Qt::black);

    // OpenGL
    glShadeModel(GL_SMOOTH);
}

void LangmuirViewer::draw()
{
}

void LangmuirViewer::postDraw()
{
    QGLViewer::postDraw();
    m_cornerAxis->render();
}

void LangmuirViewer::animate()
{
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
