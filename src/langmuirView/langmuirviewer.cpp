#include "langmuirviewer.h"
#include "keyvalueparser.h"
#include "openclhelper.h"
#include "checkpointer.h"
#include "chargeagent.h"
#include "parameters.h"
#include "simulation.h"
#include "cubicgrid.h"
#include "world.h"

#include <QGridLayout>
#include <QTextEdit>
#include <QLabel>

#include <QErrorMessage>
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
    m_error = new QErrorMessage(this);
    m_error->setWindowTitle("Langmuir");
    m_error->setMinimumSize(640, 512);

    m_simulation = NULL;
    m_world = NULL;

    m_boxThickness = 1.0;

    QGLFormat format;
    format.setVersion(4, 0);
    setFormat(format);

    qDebug("langmuir: OpenGL %d.%d", context()->format().majorVersion(), context()->format().minorVersion());
}

QMatrix4x4& LangmuirViewer::getMVP()
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

void LangmuirViewer::updateElectronCloud()
{
    if (m_electons != NULL && m_world != NULL) {

        if (m_electons->vertices().size() < 3 * m_world->numElectronAgents()) {
            qFatal("langmuir: electron VBO too small");
        }

        int j = 0;
        for (int i = 0; i < m_world->numElectronAgents(); i++) {
            int s = m_world->electrons().at(i)->getCurrentSite();
            m_electons->vertices()[j + 0] = m_world->electronGrid().getIndexX(s) - m_gridHalfX + 0.5;
            m_electons->vertices()[j + 1] = m_world->electronGrid().getIndexY(s) - m_gridHalfY + 0.5;
            m_electons->vertices()[j + 2] = m_world->electronGrid().getIndexZ(s) - m_gridHalfZ + 0.5;
            j += 3;
        }
        m_electons->setMaxRender(m_world->numElectronAgents());
        m_electons->updateVBO();
    }
}

void LangmuirViewer::updateDefectCloud()
{
    if (m_defects != NULL && m_world != NULL) {

        if (m_defects->vertices().size() < 3 * m_world->numDefects()) {
            qFatal("langmuir: defect VBO too small");
        }

        int j = 0;
        for (int i = 0; i < m_world->numDefects(); i++) {
            int s = m_world->defectSiteIDs().at(i);
            m_defects->vertices()[j + 0] = m_world->electronGrid().getIndexX(s) - m_gridHalfX + 0.5;
            m_defects->vertices()[j + 1] = m_world->electronGrid().getIndexY(s) - m_gridHalfY + 0.5;
            m_defects->vertices()[j + 2] = m_world->electronGrid().getIndexZ(s) - m_gridHalfZ + 0.5;
            j += 3;
        }
        m_defects->setMaxRender(m_world->numDefects());
        m_defects->updateVBO();
    }
}

void LangmuirViewer::updateHoleCloud()
{
    if (m_holes != NULL && m_world != NULL) {

        if (m_holes->vertices().size() < 3 * m_world->numHoleAgents()) {
            qFatal("langmuir: hole VBO too small");
        }

        int j = 0;
        for (int i = 0; i < m_world->numHoleAgents(); i++) {
            int s = m_world->holes().at(i)->getCurrentSite();
            m_holes->vertices()[j + 0] = m_world->holeGrid().getIndexX(s) - m_gridHalfX + 0.5;
            m_holes->vertices()[j + 1] = m_world->holeGrid().getIndexY(s) - m_gridHalfY + 0.5;
            m_holes->vertices()[j + 2] = m_world->holeGrid().getIndexZ(s) - m_gridHalfZ + 0.5;
            j += 3;
        }
        m_holes->setMaxRender(m_world->numHoleAgents());
        m_holes->updateVBO();
    }
}

void LangmuirViewer::initGeometry()
{
    unsigned int pointsE = 0;
    unsigned int pointsH = 0;
    unsigned int pointsD = 0;

    unsigned int renderE = 0;
    unsigned int renderH = 0;
    unsigned int renderD = 0;

    m_gridX = 0;
    m_gridY = 0;
    m_gridZ = 0;

    if (m_world != NULL) {
        pointsE = m_world->maxElectronAgents();
        pointsH = m_world->maxHoleAgents();
        pointsD = m_world->maxDefects();

        renderE = m_world->numElectronAgents();
        renderH = m_world->numHoleAgents();
        renderD = m_world->numDefects();

        m_gridX = m_world->parameters().gridX;
        m_gridY = m_world->parameters().gridY;
        m_gridZ = m_world->parameters().gridZ;
    }
    m_gridHalfX = 0.5 * m_gridX;
    m_gridHalfY = 0.5 * m_gridY;
    m_gridHalfZ = 0.5 * m_gridZ;

    float sceneRadius = 0.5 * sqrt(m_gridX * m_gridX + m_gridY * m_gridY + m_gridZ * m_gridZ);
    if (sceneRadius < 5) {
        sceneRadius = 5;
    }
    setSceneRadius(sceneRadius);
    showEntireScene();

    if (m_grid != NULL) {
        m_grid->setDimensions(m_gridX, m_gridY, m_gridZ);
    }

    if (m_electons != NULL) {
        m_electons->setMaxPoints(pointsE);
        m_electons->setMaxRender(renderE);
    }
    updateElectronCloud();

    if (m_defects != NULL) {
        m_defects->setMaxPoints(pointsD);
        m_defects->setMaxRender(renderD);
    }
    updateDefectCloud();

    if (m_holes != NULL) {
        m_holes->setMaxPoints(pointsH);
        m_holes->setMaxRender(renderH);
    }
    updateHoleCloud();

    if (m_baseBox != NULL)
    {
        m_baseBox->setXSize(m_gridX);
        m_baseBox->setYSize(m_gridY);
        m_baseBox->setZSize(m_boxThickness);
    }

    if (m_trapBox != NULL)
    {
        if (m_world != NULL)
        {
            if (m_world->numTraps() > 0)
            {
                QImage image;
                drawTraps(image, m_baseBox->getColor(), Qt::black);

                m_trapBox->loadImage(image);
                m_trapBox->showImage(true);
            }
        }
        else
        {
            m_trapBox->showImage(false);
        }
    }

    if (m_leftBox != NULL)
    {
        m_leftBox->setXSize(m_boxThickness);
        m_leftBox->setYSize(m_gridY);
        m_leftBox->setZSize(m_gridZ);
    }

    if (m_rightBox != NULL)
    {
        m_rightBox->setXSize(m_boxThickness);
        m_rightBox->setYSize(m_gridY);
        m_rightBox->setZSize(m_gridZ);
    }
}

void LangmuirViewer::init()
{
    // Corner Axis
    m_cornerAxis = new CornerAxis(*this, this);
    m_cornerAxis->setVisible(false);
    m_cornerAxis->makeConnections();

    // Grid
    m_grid = new Grid(*this, this);
    m_grid->setVisible(false);
    m_grid->makeConnections();

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

    // Base
    m_trapBox = new Box(*this, this);
    m_trapBox->setColor(Qt::blue);
    m_trapBox->setFaces(Box::Front);
    m_trapBox->setVisible(true);
    m_trapBox->makeConnections();

    m_baseBox = new Box(*this, this);
    m_baseBox->setColor(Qt::blue);
    m_baseBox->setFaces(Box::Back|Box::North|Box::South|Box::East|Box::West);
    m_baseBox->setVisible(true);
    m_baseBox->makeConnections();

    connect(m_baseBox, SIGNAL(colorChanged(QColor)), m_trapBox, SLOT(setColor(QColor)));
    connect(m_baseBox, SIGNAL(xSizeChanged(double)), m_trapBox, SLOT(setXSize(double)));
    connect(m_baseBox, SIGNAL(ySizeChanged(double)), m_trapBox, SLOT(setYSize(double)));
    connect(m_baseBox, SIGNAL(zSizeChanged(double)), m_trapBox, SLOT(setZSize(double)));

    // Left Electrode
    m_leftBox = new Box(*this, this);
    m_leftBox->setColor(Qt::red);
    m_leftBox->setFaces(Box::All);
    m_leftBox->setVisible(true);
    m_leftBox->makeConnections();

    // Left Electrode
    m_rightBox = new Box(*this, this);
    m_rightBox->setColor(Qt::red);
    m_rightBox->setFaces(Box::All);
    m_rightBox->setVisible(true);
    m_rightBox->makeConnections();

    initGeometry();

    // Light
    glEnable(GL_LIGHTING);

    // makes glColor set ambient and diffuse components of polygon
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    m_light0 = new Light(GL_LIGHT0, *this, this);
    m_light0->setAColor(QColor(128, 128, 128, 255));  // white light but weaker
    m_light0->setSColor(QColor(255, 255, 255, 255));  // white light
    m_light0->setDColor(QColor(255, 255, 255, 255));  // white light
    m_light0->setPosition(1.0, 1.0, 1.0, 0.0);
    m_light0->setEnabled(true);
    m_light0->makeConnections();

    // Background
    //setBackgroundColor(Qt::black);

    // OpenGL
    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_POINT_SMOOTH);
}

void LangmuirViewer::preDraw()
{
    QGLViewer::preDraw();
    m_light0->updatePosition();
}

void LangmuirViewer::draw()
{
    glPushMatrix();
        glTranslatef(0.0f, 0.0f, +0.05);
        m_electons->render();
        m_defects->render();
        m_holes->render();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(0.0f, 0.0f, -0.5 * m_boxThickness);
        m_grid->render();

        glPushMatrix();
            glTranslatef(0.0, 0.0, -0.05);
            m_trapBox->render();
            m_baseBox->render();
        glPopMatrix();

        glPushMatrix();
            glTranslatef(-m_gridHalfX - 0.5 * m_boxThickness, 0.0, 0.0);
            m_leftBox->render();
        glPopMatrix();

        glPushMatrix();
            glTranslatef(+m_gridHalfX + 0.5 * m_boxThickness, 0.0, 0.0);
            m_leftBox->render();
        glPopMatrix();
    glPopMatrix();

    m_light0->render();
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

    m_simulation->performIterations(m_world->parameters().iterationsPrint);
    updateElectronCloud();
    updateHoleCloud();
}

void LangmuirViewer::help() {
    emit showMessage("implement help widget!");
}

QString LangmuirViewer::helpString() const
{
    return "";
}

void LangmuirViewer::toggleOpenCL(bool on)
{
    if (m_world == NULL) {
        emit showMessage("can not enable OpenCL");
        return;
    }

    m_world->opencl().toggleOpenCL(on);
    emit isUsingOpenCL(m_world->parameters().useOpenCL);
}

void LangmuirViewer::toggleCornerAxisIsVisible()
{
    m_cornerAxis->toggleVisible();
    emit showMessage(QString("corner axis=%1").arg(m_cornerAxis->isVisible()));
}

void LangmuirViewer::toggleGridIsVisible()
{
    m_grid->toggleVisible();
    emit showMessage(QString("grid=%1").arg(m_cornerAxis->isVisible()));
}

void LangmuirViewer::load(QString fileName)
{
    if (animationIsStarted()) {
        pause();
    }

    if (fileName.isEmpty()) {
        m_error->showMessage("Can not load the simulation (no file selected)");

        emit showMessage("no file selected");
        return;
    }

    Langmuir::World *world = new Langmuir::World(fileName, -1, -1, this);
    Langmuir::Simulation *simulation = new Langmuir::Simulation(*world, world);

    world->parameters().outputIsOn = false;
    world->parameters().iterationsPrint = 1;

    if (m_world != NULL || m_simulation != NULL) {
        unload();
    }

    m_simulation = simulation;
    m_world = world;

    initGeometry();

    QFileInfo info(fileName);
    emit showMessage(QString("loaded file: %1").arg(info.fileName()));

    emit isUsingOpenCL(m_world->parameters().useOpenCL);
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
        m_error->showMessage("Can not save the simulation (no simulation loaded)");

        emit showMessage("can not save simulation");

        return;
    }
}

void LangmuirViewer::unload()
{
    if (animationIsStarted()) {
        pause();
    }

    if (m_simulation == NULL || m_world == NULL) {

        m_error->showMessage("Can not delete the simulation (no simulation loaded)");

        emit showMessage("can not delete simulation");

        return;
    }

    if (m_simulation != NULL) {
        m_simulation->deleteLater();
    }

    if (m_world != NULL) {
        m_world->deleteLater();
    }

    m_simulation = NULL;
    m_world = NULL;

    initGeometry();
    updateGL();

    emit showMessage("simulation deleted");
}

void LangmuirViewer::reset() {

    // pause the simulation
    if (animationIsStarted()) {
        pause();
    }

    // check if there is a simulation
    if (m_world == NULL || m_simulation == NULL) {

        m_error->showMessage("Can not reset the simulation (no simulation loaded)");

        emit showMessage("can not reset simulation");

        return;
    }

    // copy the simulation parameters
    Langmuir::SimulationParameters parameters = m_world->parameters();

    // decide on random seed
    QMessageBox::StandardButton answer = QMessageBox::question(this, "Random seed",
        "Use the same random seed?", QMessageBox::Cancel|QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

    switch (answer)
    {
        case QMessageBox::Yes:
        {
            break;
        }
        case QMessageBox::No:
        {
            parameters.randomSeed = 0;
            break;
        }
        case QMessageBox::Cancel: default:
        {
            m_error->showMessage("Can not reset the simulation (aborted)");

            emit showMessage("aborted simulation reset");

            return;

            break;
        }
    }

    parameters.simulationStart = QDateTime::currentDateTime();
    parameters.outputIsOn      = false;
    parameters.iterationsPrint = 1;
    parameters.currentStep     = 0;

    Langmuir::World *world = new Langmuir::World(parameters, -1, -1, this);
    Langmuir::Simulation *simulation = new Langmuir::Simulation(*world, world);

    unload();

    m_simulation = simulation;
    m_world = world;

    initGeometry();

    emit showMessage("reset simulation");
    emit isUsingOpenCL(m_world->parameters().useOpenCL);
}

void LangmuirViewer::resetCamera()
{
    camera()->frame()->stopSpinning();
    camera()->setUpVector(qglviewer::Vec(0, 1, 0));
    camera()->setViewDirection(qglviewer::Vec(0, 0, -1));
    camera()->showEntireScene();
    updateGL();
}

void LangmuirViewer::pause()
{
    if (animationIsStarted()) {
        stopAnimation();
        emit showMessage("simulation is paused");
    }
    emit isAnimated(animationIsStarted());
}

void LangmuirViewer::play()
{
    if (animationIsStarted()) {
    } else {
        startAnimation();
        emit showMessage("simulation is playing");
    }
    emit isAnimated(animationIsStarted());
}

void LangmuirViewer::showParameters()
{
    // pause the simulation
    if (animationIsStarted()) {
        pause();
    }

    // check if there is a simulation
    if (m_world == NULL || m_simulation == NULL) {

        m_error->showMessage("Can not show simulation parameters (no simulation loaded)");

        emit showMessage("can not show simulation parameters");

        return;
    }

    // get variables
    const QStringList& keys = m_world->keyValueParser().getOrderedNames();
    const QMap<QString,Langmuir::Variable*>& variableMap = m_world->keyValueParser().getVariableMap();

    // create HTML table
    QString title = "<td><b><font face=\"courier\" color=\"%1\">%2<font></b></td>";
    QString label = "<td><b><font face=\"courier\">%1<font></b></td>";
    QString value = "<td><b><font face=\"courier\">%1<font></b></td>";

    QString parameters = "<table>";
    parameters += "<tr>";
    parameters += title.arg("red").arg("KEY");
    parameters += title.arg("blue").arg("VALUE");
    parameters += "</tr>";

    foreach(QString key, keys)
    {
        Langmuir::Variable *variable = variableMap[key];
        if (!variable->isConstant()) {
            parameters += "<tr>";
            parameters += label.arg(variable->key());
            parameters += value.arg(variable->value());
            parameters += "</tr>";
        }
    }
    parameters += "<table/>";

    m_error->showMessage(parameters);
}

void LangmuirViewer::drawLightSource(GLenum light, float scale) const
{
    drawLight(light, scale);
}

void LangmuirViewer::drawTraps(QImage &image, QColor bcolor, QColor fcolor)
{
    if (m_world == NULL || m_simulation == NULL)
    {
        return;
    }

    if (m_world->numTraps() <= 0)
    {
        return;
    }

    int xsize = m_world->parameters().gridX;
    int ysize = m_world->parameters().gridY;

    image = QImage(xsize, ysize, QImage::Format_ARGB32_Premultiplied);
    image.fill(1);

    QPainter painter;
    painter.begin(&image);

    painter.scale(1.0, -1.0);
    painter.translate(0.0, -ysize);
    painter.setWindow(0, 0, xsize, ysize);

    painter.fillRect(QRect(0,0,xsize,ysize), bcolor);
    painter.setPen(fcolor);

    foreach(int s, m_world->trapSiteIDs())
    {
        int z = m_world->electronGrid().getIndexZ(s);

        if (z == 0)
        {
            int x = m_world->electronGrid().getIndexX(s);
            int y = m_world->electronGrid().getIndexY(s);

            painter.drawPoint(x, y);
        }
    }

    painter.end();

    int scale = 5;
    image = image.scaled(scale * image.width(), scale * image.height(), Qt::KeepAspectRatioByExpanding);
}
