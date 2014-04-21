#include "langmuirviewer.h"

LangmuirViewer::LangmuirViewer(QWidget *parent) :
    QGLViewer(parent)
{
}

void LangmuirViewer::init()
{
    setAxisIsDrawn();
}

void LangmuirViewer::draw()
{
}

void LangmuirViewer::animate()
{
}

QString LangmuirViewer::helpString() const
{
    return "";
}
