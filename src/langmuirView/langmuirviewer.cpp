#include "langmuirviewer.h"
#include <QDebug>

LangmuirViewer::LangmuirViewer(QWidget *parent) :
    QGLViewer(parent)
{
}

void LangmuirViewer::init()
{
}

void LangmuirViewer::draw()
{
}

void LangmuirViewer::animate()
{
    qDebug() << "1";
}

QString LangmuirViewer::helpString() const
{
    return "";
}
