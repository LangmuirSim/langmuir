#include "langmuirviewer.h"
#include <QColor>
#include <QDebug>

LangmuirViewer::LangmuirViewer(QWidget *parent) :
    QGLViewer(parent), cornerAxisIsDrawn_(false)
{
}

void LangmuirViewer::init()
{
    qDebug() << axisIsDrawn();
}

void LangmuirViewer::draw()
{
}

void LangmuirViewer::postDraw()
{
    QGLViewer::postDraw();
    if (cornerAxisIsDrawn_) {
        drawCornerAxis();
    }
}

void LangmuirViewer::animate()
{
    qDebug() << "1";
}

QString LangmuirViewer::helpString() const
{
    return "";
}

bool LangmuirViewer::cornerAxisIsDrawn()
{
    return cornerAxisIsDrawn_;
}

void LangmuirViewer::toggleCornerAxisIsDrawn()
{
    if (cornerAxisIsDrawn_) {
        cornerAxisIsDrawn_ = false;
    }
    else {
        cornerAxisIsDrawn_ = true;
    }
    emit cornerAxisIsDrawnChanged(cornerAxisIsDrawn_);
}

void LangmuirViewer::setCornerAxisIsDrawn(bool draw)
{
    cornerAxisIsDrawn_ = draw;
    emit cornerAxisIsDrawnChanged(cornerAxisIsDrawn_);
}

void LangmuirViewer::drawCornerAxis()
{    
    // original viewport saved
    int view[4], sbox[4];
    glGetIntegerv(GL_VIEWPORT, view);
    glGetIntegerv(GL_SCISSOR_BOX, sbox);

    // view port is placed at lower left
    const int size = 150;
    glViewport(0, 0, size, size);
    glScissor(0, 0, size, size);

    // axis appears on top
    glClear(GL_DEPTH_BUFFER_BIT);

    // draw axis
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMultMatrixd(camera()->orientation().inverse().matrix());

    // lighting and color saved
    GLboolean lighting, colorMaterial;
    glGetBooleanv(GL_LIGHTING, &lighting);
    glGetBooleanv(GL_COLOR_MATERIAL, &colorMaterial);

    // enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);

    // axis parameters
    float length = 1.0;
    float color[4];

    // x-axis
    color[0] = 0.7f;  color[1] = 0.7f;  color[2] = 1.0f;  color[3] = 1.0f;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glPushMatrix();
    glRotatef(0.0, 0.0, 1.0, 0.0);
    QGLViewer::drawArrow(length, 0.01*length);
    glPopMatrix();

    // y-axis
    color[0] = 1.0f;  color[1] = 0.7f;  color[2] = 0.7f;  color[3] = 1.0f;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glPushMatrix();
    glRotatef(90.0, 0.0, 1.0, 0.0);
    QGLViewer::drawArrow(length, 0.01*length);
    glPopMatrix();

    // z-axis
    color[0] = 0.7f;  color[1] = 1.0f;  color[2] = 0.7f;  color[3] = 1.0f;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glPushMatrix();
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    QGLViewer::drawArrow(length, 0.01*length);
    glPopMatrix();

    // restore
    if (colorMaterial) {
        glEnable(GL_COLOR_MATERIAL);
    }
    else {
        glDisable(GL_COLOR_MATERIAL);
    }

    if (lighting) {
        glEnable(GL_LIGHTING);
    }
    else {
        glDisable(GL_LIGHTING);
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_LIGHTING);

    glScissor(sbox[0],sbox[1],sbox[2],sbox[3]);
    glViewport(view[0],view[1],view[2],view[3]);
}
