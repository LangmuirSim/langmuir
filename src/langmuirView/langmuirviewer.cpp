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
}

void LangmuirViewer::load(QString fileName)
{
    if (!fileName.isEmpty()) {
        return;
    }
}

void LangmuirViewer::unload()
{

}
