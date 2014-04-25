#include "langmuirviewer.h"
#include <QColor>
#include <QDebug>

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

void LangmuirViewer::open()
{
    qDebug() << "open called";
}
