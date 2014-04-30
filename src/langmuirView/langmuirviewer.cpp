#include "langmuirviewer.h"
#include "simulation.h"
#include "world.h"

#include <QFileDialog>
#include <QString>
#include <QColor>
#include <QDebug>
#include <QDir>

LangmuirViewer::LangmuirViewer(QWidget *parent) :
    QGLViewer(parent)
{
    m_simulation = NULL;
    m_world = NULL;
}

void LangmuirViewer::init()
{
    m_cornerAxis = new CornerAxis(*this, this);
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

    unload();

    m_simulation = simulation;
    m_world = world;
}

void LangmuirViewer::unload()
{
    if (m_simulation != NULL) {
        m_simulation->deleteLater();
    }

    if (m_world != NULL) {
        m_world->deleteLater();
    }

    m_simulation = NULL;
    m_world = NULL;
}
