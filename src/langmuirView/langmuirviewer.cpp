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
#include <QSettings>
#include <QString>
#include <QColor>
#include <QDebug>
#include <QDir>

#include <QtConcurrent>
#include <QFuture>

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
    m_trapColor = Qt::black;
    m_isoSurface = NULL;

    m_canCalculateIsoSurface = false;

    QGLFormat format;
    format.setVersion(4, 0);
    setFormat(format);

    qDebug("langmuir: OpenGL %d.%d", context()->format().majorVersion(), context()->format().minorVersion());
}

LangmuirViewer::~LangmuirViewer()
{
}

QMatrix4x4& LangmuirViewer::getModelViewProjectionMatrix()
{
    static GLdouble matrix[16];
    camera()->getModelViewProjectionMatrix(matrix);

    static QMatrix4x4 qmatrix;

    qmatrix(0, 0) = matrix[0];
    qmatrix(1, 0) = matrix[1];
    qmatrix(2, 0) = matrix[2];
    qmatrix(3, 0) = matrix[3];

    qmatrix(0, 1) = matrix[4];
    qmatrix(1, 1) = matrix[5];
    qmatrix(2, 1) = matrix[6];
    qmatrix(3, 1) = matrix[7];

    qmatrix(0, 2) = matrix[8];
    qmatrix(1, 2) = matrix[9];
    qmatrix(2, 2) = matrix[10];
    qmatrix(3, 2) = matrix[11];

    qmatrix(0, 3) = matrix[12];
    qmatrix(1, 3) = matrix[13];
    qmatrix(2, 3) = matrix[14];
    qmatrix(3, 3) = matrix[15];

    return qmatrix;
}

QMatrix4x4& LangmuirViewer::getProjectionMatrix()
{
    static GLdouble matrix[16];
    camera()->getProjectionMatrix(matrix);

    static QMatrix4x4 qmatrix;

    qmatrix(0, 0) = matrix[0];
    qmatrix(1, 0) = matrix[1];
    qmatrix(2, 0) = matrix[2];
    qmatrix(3, 0) = matrix[3];

    qmatrix(0, 1) = matrix[4];
    qmatrix(1, 1) = matrix[5];
    qmatrix(2, 1) = matrix[6];
    qmatrix(3, 1) = matrix[7];

    qmatrix(0, 2) = matrix[8];
    qmatrix(1, 2) = matrix[9];
    qmatrix(2, 2) = matrix[10];
    qmatrix(3, 2) = matrix[11];

    qmatrix(0, 3) = matrix[12];
    qmatrix(1, 3) = matrix[13];
    qmatrix(2, 3) = matrix[14];
    qmatrix(3, 3) = matrix[15];

    return qmatrix;
}

QMatrix4x4& LangmuirViewer::getOpenGLModelViewMatrix()
{
    static GLfloat matrix[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);

    static QMatrix4x4 qmatrix;

    qmatrix(0, 0) = matrix[0];
    qmatrix(1, 0) = matrix[1];
    qmatrix(2, 0) = matrix[2];
    qmatrix(3, 0) = matrix[3];

    qmatrix(0, 1) = matrix[4];
    qmatrix(1, 1) = matrix[5];
    qmatrix(2, 1) = matrix[6];
    qmatrix(3, 1) = matrix[7];

    qmatrix(0, 2) = matrix[8];
    qmatrix(1, 2) = matrix[9];
    qmatrix(2, 2) = matrix[10];
    qmatrix(3, 2) = matrix[11];

    qmatrix(0, 3) = matrix[12];
    qmatrix(1, 3) = matrix[13];
    qmatrix(2, 3) = matrix[14];
    qmatrix(3, 3) = matrix[15];

    return qmatrix;
}

QMatrix4x4& LangmuirViewer::getOpenGLProjectionMatrix()
{
    static GLfloat matrix[16];
    glGetFloatv(GL_PROJECTION_MATRIX, matrix);

    static QMatrix4x4 qmatrix;

    qmatrix(0, 0) = matrix[0];
    qmatrix(1, 0) = matrix[1];
    qmatrix(2, 0) = matrix[2];
    qmatrix(3, 0) = matrix[3];

    qmatrix(0, 1) = matrix[4];
    qmatrix(1, 1) = matrix[5];
    qmatrix(2, 1) = matrix[6];
    qmatrix(3, 1) = matrix[7];

    qmatrix(0, 2) = matrix[8];
    qmatrix(1, 2) = matrix[9];
    qmatrix(2, 2) = matrix[10];
    qmatrix(3, 2) = matrix[11];

    qmatrix(0, 3) = matrix[12];
    qmatrix(1, 3) = matrix[13];
    qmatrix(2, 3) = matrix[14];
    qmatrix(3, 3) = matrix[15];

    return qmatrix;
}

Langmuir::Random& LangmuirViewer::random()
{
    return m_random;
}

void LangmuirViewer::updateElectronCloud()
{
    if (m_electrons != NULL && m_world != NULL) {

        if (m_electrons->vertices().size() < 3 * m_world->numElectronAgents()) {
            qFatal("langmuir: electron VBO too small");
        }

        int j = 0;
        for (int i = 0; i < m_world->numElectronAgents(); i++) {
            int s = m_world->electrons().at(i)->getCurrentSite();
            m_electrons->vertices()[j + 0] = m_world->electronGrid().getIndexX(s) - m_gridHalfX + 0.5;
            m_electrons->vertices()[j + 1] = m_world->electronGrid().getIndexY(s) - m_gridHalfY + 0.5;
            m_electrons->vertices()[j + 2] = m_world->electronGrid().getIndexZ(s) - m_gridHalfZ + 0.5;
            j += 3;
        }
        m_electrons->setMaxRender(m_world->numElectronAgents());
        m_electrons->updateVBO();
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

    if (m_electrons != NULL) {
        m_electrons->setMaxPoints(pointsE);
        m_electrons->setMaxRender(renderE);
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

    initTraps();

    if (m_lBox != NULL)
    {
        m_lBox->setXSize(2 * m_boxThickness);
        m_lBox->setYSize(m_gridY);
        m_lBox->setZSize(m_gridZ + m_boxThickness);
    }

    if (m_rBox != NULL)
    {
        m_rBox->setXSize(2 * m_boxThickness);
        m_rBox->setYSize(m_gridY);
        m_rBox->setZSize(m_gridZ + m_boxThickness);
    }

    if (m_light0 != NULL)
    {
        m_light0->setPosition(0, 0, 1.25 * m_gridZ, 1.0);
    }

    if (m_trapMesh != NULL)
    {
        m_trapMesh->clear();
        m_trapMesh->setVisible(false);
    }

    if (m_gridZ > 1) {
        setCanCalculateIsoSurface(true);
    } else {
        setCanCalculateIsoSurface(false);
    }
}

void LangmuirViewer::initTraps()
{
    if (m_trapBox != NULL && m_baseBox != NULL)
    {
        if (m_world != NULL)
        {
            if (m_world->numTraps() > 0)
            {
                QImage image;
                drawTraps(image, m_baseBox->getColor(), m_trapColor);

                m_trapBox->loadImage(image);
                m_trapBox->showImage(true);
            }
        }
        else
        {
            m_trapBox->showImage(false);
        }
    }
}

void LangmuirViewer::generateIsoSurface(float value)
{
    if (!m_canCalculateIsoSurface) {
        emit showMessage("can not calculate isosurface!");
        return;
    }

    emit canCalculateIsoSurface(false);

    int xsize = m_world->parameters().gridX;
    int ysize = m_world->parameters().gridY;
    int zsize = m_world->parameters().gridZ;

    if (m_isoSurface == NULL)
    {
        m_isoSurface = new MarchingCubes::Isosurface(this);
        connect(m_isoSurface, SIGNAL(done()), this, SLOT(updateTrapMesh()));
        connect(m_isoSurface, SIGNAL(progress(int)), this, SIGNAL(isoSurfaceProgress(int)));
    }

    MarchingCubes::scalar_field& scalar = m_isoSurface->createScalarField(xsize, ysize, zsize, 1.0);
    m_isoSurface->setIsoValue(value);

    foreach (int site, m_world->trapSiteIDs())
    {
        int x = m_world->electronGrid().getIndexX(site);
        int y = m_world->electronGrid().getIndexY(site);
        int z = m_world->electronGrid().getIndexZ(site);
        scalar[x][y][z] = 1.0;
    }

    QtConcurrent::run(m_isoSurface, &MarchingCubes::Isosurface::generate);

    emit showMessage("calculating isosurface");
}

void LangmuirViewer::updateTrapMesh()
{
    if (m_isoSurface == NULL)
    {
        emit showMessage("can not update trap mesh!");
        return;
    }

    emit showMessage("isosurface done!");

    m_trapMesh->setMesh(m_isoSurface->vertices(), m_isoSurface->normals(), m_isoSurface->indices());

    qDebug("langmuir: %d vertices", m_isoSurface->vertices().size());
    qDebug("langmuir: %d normals", m_isoSurface->normals().size());
    qDebug("langmuir: %d indices", m_isoSurface->indices().size());

    m_isoSurface->clear();

    m_trapMesh->setVisible(true);

    emit canCalculateIsoSurface(true);
}

void LangmuirViewer::resetSettings()
{
    QFileInfo info(QDir::current().absoluteFilePath("config.ini"));
    if (info.exists()) {
        loadSettings(info.absoluteFilePath());
        return;
    } else {
        QSettings settings;
        getSettings(settings);
    }
}

void LangmuirViewer::setCanCalculateIsoSurface(bool enabled)
{
    if (enabled != m_canCalculateIsoSurface)
    {
        m_canCalculateIsoSurface = enabled;
        emit canCalculateIsoSurface(enabled);
    }
}

void LangmuirViewer::init()
{
    // Corner Axis
    m_cornerAxis = new CornerAxis(*this, this);
    m_cornerAxis->makeConnections();

    // Grid
    m_grid = new Grid(*this, this);
    m_grid->makeConnections();

    // Electrons
    m_electrons = new PointCloud(*this, this);
    m_electrons->makeConnections();

    // Defects
    m_defects = new PointCloud(*this, this);
    m_defects->makeConnections();

    // Holes
    m_holes = new PointCloud(*this, this);
    m_holes->makeConnections();

    // Base
    m_trapBox = new Box(*this, this);
    m_trapBox->setFaces(Box::Front);
    m_trapBox->makeConnections();

    m_baseBox = new Box(*this, this);
    m_baseBox->setFaces(Box::Back|Box::North|Box::South|Box::East|Box::West);
    m_baseBox->makeConnections();

    connect(m_baseBox, SIGNAL(colorChanged(QColor)), m_trapBox, SLOT(setColor(QColor)));
    connect(m_baseBox, SIGNAL(xSizeChanged(double)), m_trapBox, SLOT(setXSize(double)));
    connect(m_baseBox, SIGNAL(ySizeChanged(double)), m_trapBox, SLOT(setYSize(double)));
    connect(m_baseBox, SIGNAL(zSizeChanged(double)), m_trapBox, SLOT(setZSize(double)));

    // Left Electrode
    m_lBox = new Box(*this, this);
    m_lBox->setFaces(Box::All);
    m_lBox->makeConnections();

    // Left Electrode
    m_rBox = new Box(*this, this);
    m_rBox->setFaces(Box::All);
    m_rBox->makeConnections();

    // Mesh
    m_trapMesh = new Mesh(*this, this);
    m_trapMesh->makeConnections();

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

    initGeometry();

    // Background
    setBackgroundColor(QColor(32, 32, 32));

    // OpenGL
    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_POINT_SMOOTH);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    // OpenGL is done
    emit openGLInitFinished();
}

void LangmuirViewer::preDraw()
{
    QGLViewer::preDraw();
    m_light0->updatePosition();
}

void LangmuirViewer::draw()
{
    glPushMatrix();
        glTranslatef(0.0, 0.0, 0.5 * m_boxThickness + 0.1);
        m_electrons->render();
        m_defects->render();
        m_holes->render();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(0.0, 0.0, -m_gridHalfZ + 0.5 * m_boxThickness);
        glPushMatrix();
            glTranslatef(0.0f, 0.0f, 0.5 * m_boxThickness + 0.1);
            m_grid->render();
        glPopMatrix();
        m_trapBox->render();
        m_baseBox->render();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(-m_gridHalfX - m_boxThickness, 0.0, 0.0);
        m_lBox->render();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(+m_gridHalfX + m_boxThickness, 0.0, 0.0);
        m_rBox->render();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(-m_gridHalfX, -m_gridHalfY, -m_gridHalfZ);
        m_trapMesh->render();
    glPopMatrix();
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

    emit currentStepChanged(m_world->parameters().currentStep);
}

void LangmuirViewer::help() {
    emit showMessage("implement help widget!");
}

QString LangmuirViewer::helpString() const
{
    return "";
}

void LangmuirViewer::loadSettings(QString fileName)
{
    if (animationIsStarted()) {
        pause();
    }

    if (fileName.isEmpty()) {
        m_error->showMessage("Can not load settings (no file selected)");

        emit showMessage("no file selected");
        return;
    }

    QSettings settings(fileName, QSettings::IniFormat);
    getSettings(settings);

    emit showMessage("loaded settings");
}

void LangmuirViewer::saveSettings(QString fileName)
{
    if (animationIsStarted()) {
        pause();
    }

    if (fileName.isEmpty()) {
        m_error->showMessage("Can not save settings (no file selected)");

        emit showMessage("no file selected");
        return;
    }

    QSettings settings(fileName, QSettings::IniFormat);
    setSettings(settings);
    settings.sync();

    emit showMessage("saved settings");
}

void LangmuirViewer::setSettings(QSettings &settings)
{
    if (animationIsStarted()) {
        pause();
    }

    // electrons
    {
        settings.beginGroup("electrons");
        settings.setValue("mode"     , (int   )m_electrons->getMode());
        settings.setValue("visible"  , (bool  )m_electrons->isVisible());
        settings.setValue("color_r"  , (int   )m_electrons->getColor().red());
        settings.setValue("color_g"  , (int   )m_electrons->getColor().green());
        settings.setValue("color_b"  , (int   )m_electrons->getColor().blue());
        settings.setValue("pointsize", (double)m_electrons->getPointSize());
        settings.endGroup();
    }

    // holes
    {
        settings.beginGroup("holes");
        settings.setValue("mode"     , (int   )m_holes->getMode());
        settings.setValue("visible"  , (bool  )m_holes->isVisible());
        settings.setValue("color_r"  , (int   )m_holes->getColor().red());
        settings.setValue("color_g"  , (int   )m_holes->getColor().green());
        settings.setValue("color_b"  , (int   )m_holes->getColor().blue());
        settings.setValue("pointsize", (double)m_holes->getPointSize());
        settings.endGroup();
    }

    // defects
    {
        settings.beginGroup("defects");
        settings.setValue("mode"     , (int   )m_defects->getMode());
        settings.setValue("visible"  , (bool  )m_defects->isVisible());
        settings.setValue("color_r"  , (int   )m_defects->getColor().red());
        settings.setValue("color_g"  , (int   )m_defects->getColor().green());
        settings.setValue("color_b"  , (int   )m_defects->getColor().blue());
        settings.setValue("pointsize", (double)m_defects->getPointSize());
        settings.endGroup();
    }

    // corner axis
    {
        settings.beginGroup("corneraxis");
        settings.setValue("xcolor_r"  , (int   )m_cornerAxis->getXColor().red());
        settings.setValue("xcolor_g"  , (int   )m_cornerAxis->getXColor().green());
        settings.setValue("xcolor_b"  , (int   )m_cornerAxis->getXColor().blue());
        settings.setValue("ycolor_r"  , (int   )m_cornerAxis->getYColor().red());
        settings.setValue("ycolor_g"  , (int   )m_cornerAxis->getYColor().green());
        settings.setValue("ycolor_b"  , (int   )m_cornerAxis->getYColor().blue());
        settings.setValue("zcolor_r"  , (int   )m_cornerAxis->getZColor().red());
        settings.setValue("zcolor_g"  , (int   )m_cornerAxis->getZColor().green());
        settings.setValue("zcolor_b"  , (int   )m_cornerAxis->getZColor().blue());
        settings.setValue("length"    , (double)m_cornerAxis->getLength());
        settings.setValue("radius"    , (double)m_cornerAxis->getRadius());
        settings.setValue("shift"     , (int   )m_cornerAxis->getShift());
        settings.setValue("size"      , (int   )m_cornerAxis->getSize());
        settings.setValue("location"  , (int   )m_cornerAxis->getLocation());
        settings.setValue("visible"   , (bool  )m_cornerAxis->isVisible());
        settings.endGroup();
    }

    // base
    {
        settings.beginGroup("base");
        settings.setValue("color_r"  , (int   )m_baseBox->getColor().red());
        settings.setValue("color_g"  , (int   )m_baseBox->getColor().green());
        settings.setValue("color_b"  , (int   )m_baseBox->getColor().blue());
        settings.setValue("visible"  , (bool  )m_baseBox->isVisible());
        settings.endGroup();
    }

    // traps
    {
        settings.beginGroup("traps");
        settings.setValue("color_r"  , (int   )m_trapColor.red());
        settings.setValue("color_g"  , (int   )m_trapColor.green());
        settings.setValue("color_b"  , (int   )m_trapColor.blue());
        settings.setValue("visible"  , (bool  )m_trapBox->imageIsOn());
        settings.endGroup();
    }

    // mesh
    {
        settings.beginGroup("mesh");
        settings.setValue("mode"    , (int   )m_trapMesh->getMode());
        settings.setValue("acolor_r", (int   )m_trapMesh->getColorA().red());
        settings.setValue("acolor_g", (int   )m_trapMesh->getColorA().green());
        settings.setValue("acolor_b", (int   )m_trapMesh->getColorA().blue());
        settings.setValue("acolor_a", (int   )m_trapMesh->getColorA().alpha());
        settings.setValue("bcolor_r", (int   )m_trapMesh->getColorB().red());
        settings.setValue("bcolor_g", (int   )m_trapMesh->getColorB().green());
        settings.setValue("bcolor_b", (int   )m_trapMesh->getColorB().blue());
        settings.setValue("bcolor_a", (int   )m_trapMesh->getColorB().alpha());
        settings.setValue("visible" , (bool  )m_trapMesh->isVisible());
        settings.endGroup();
    }

    // electrodes
    {
        settings.beginGroup("electrodes");
        settings.setValue("color_r"  , (int   )m_lBox->getColor().red());
        settings.setValue("color_g"  , (int   )m_lBox->getColor().green());
        settings.setValue("color_b"  , (int   )m_lBox->getColor().blue());
        settings.setValue("visible"  , (bool  )m_lBox->isVisible());
        settings.endGroup();
    }

    // grid
    {
        settings.beginGroup("grid");
        settings.setValue("color_r"  , (int   )m_grid->getColor().red());
        settings.setValue("color_g"  , (int   )m_grid->getColor().green());
        settings.setValue("color_b"  , (int   )m_grid->getColor().blue());
        settings.setValue("visible"  , (bool  )m_grid->isVisible());
        settings.endGroup();
    }

    // background
    {
        QColor color = backgroundColor();
        settings.beginGroup("background");
        settings.setValue("color_r"  , (int   )color.red());
        settings.setValue("color_g"  , (int   )color.green());
        settings.setValue("color_b"  , (int   )color.blue());
        settings.endGroup();
    }
}

void LangmuirViewer::getSettings(QSettings &settings)
{
    if (animationIsStarted()) {
        pause();
    }

    // electrons
    {
        settings.beginGroup("electrons");

        // mode
        PointCloud::Mode mode = PointCloud::Mode(settings.value("mode", PointCloud::Cubes).toInt());

        // visible
        bool visible = settings.value("visible", true).toBool();

        // color
        int color_r = settings.value("color_r", 255).toInt();
        int color_g = settings.value("color_g", 255).toInt();
        int color_b = settings.value("color_b", 255).toInt();
        QColor color = QColor::fromRgb(color_r, color_g, color_b);

        // pointsize
        double pointsize = settings.value("pointsize", 230.0).toDouble();

        settings.endGroup();

        // update settings
        m_electrons->setMode(mode);
        m_electrons->setVisible(visible);
        m_electrons->setColor(color);
        m_electrons->setPointSize(pointsize);
    }

    // holes
    {
        settings.beginGroup("holes");

        // mode
        PointCloud::Mode mode = PointCloud::Mode(settings.value("mode", PointCloud::Cubes).toInt());

        // visible
        bool visible = settings.value("visible", true).toBool();

        // color
        int color_r = settings.value("color_r",   0).toInt();
        int color_g = settings.value("color_g",   0).toInt();
        int color_b = settings.value("color_b",   0).toInt();
        QColor color = QColor::fromRgb(color_r, color_g, color_b);

        // pointsize
        double pointsize = settings.value("pointsize", 230.0).toDouble();

        settings.endGroup();

        // update settings
        m_holes->setMode(mode);
        m_holes->setVisible(visible);
        m_holes->setColor(color);
        m_holes->setPointSize(pointsize);
    }

    // defects
    {
        settings.beginGroup("defects");

        // mode
        PointCloud::Mode mode = PointCloud::Mode(settings.value("mode", PointCloud::Cubes).toInt());

        // visible
        bool visible = settings.value("visible", true).toBool();

        // color
        int color_r = settings.value("color_r",   0).toInt();
        int color_g = settings.value("color_g", 255).toInt();
        int color_b = settings.value("color_b",   0).toInt();
        QColor color = QColor::fromRgb(color_r, color_g, color_b);

        // pointsize
        double pointsize = settings.value("pointsize", 230.0).toDouble();

        settings.endGroup();

        // update settings
        m_defects->setMode(mode);
        m_defects->setVisible(visible);
        m_defects->setColor(color);
        m_defects->setPointSize(pointsize);
    }

    // corner axis
    {
        settings.beginGroup("corneraxis");

        // location
        CornerAxis::Location location = CornerAxis::Location(settings.value("location", CornerAxis::LowerLeft).toInt());

        // visible
        bool visible = settings.value("visible", false).toBool();

        // xcolor
        int xcolor_r = settings.value("xcolor_r", 255).toInt();
        int xcolor_g = settings.value("xcolor_g",   0).toInt();
        int xcolor_b = settings.value("xcolor_b",   0).toInt();
        QColor xcolor = QColor::fromRgb(xcolor_r, xcolor_g, xcolor_b);

        // ycolor
        int ycolor_r = settings.value("ycolor_r",   0).toInt();
        int ycolor_g = settings.value("ycolor_g", 255).toInt();
        int ycolor_b = settings.value("ycolor_b",   0).toInt();
        QColor ycolor = QColor::fromRgb(ycolor_r, ycolor_g, ycolor_b);

        // zcolor
        int zcolor_r = settings.value("zcolor_r",   0).toInt();
        int zcolor_g = settings.value("zcolor_g",   0).toInt();
        int zcolor_b = settings.value("zcolor_b", 255).toInt();
        QColor zcolor = QColor::fromRgb(zcolor_r, zcolor_g, zcolor_b);

        // length
        double length = settings.value("length", 1.00).toDouble();

        // radius
        double radius = settings.value("radius", 0.01).toDouble();

        // shift
        int shift = settings.value("shift", 10).toInt();

        // size
        int size = settings.value("size", 150).toInt();

        settings.endGroup();

        // update settings
        m_cornerAxis->setLocation(location);
        m_cornerAxis->setVisible(visible);
        m_cornerAxis->setXColor(xcolor);
        m_cornerAxis->setYColor(ycolor);
        m_cornerAxis->setZColor(zcolor);
        m_cornerAxis->setLength(length);
        m_cornerAxis->setRadius(radius);
        m_cornerAxis->setShift(shift);
        m_cornerAxis->setSize(size);
    }

    // base
    {
        settings.beginGroup("base");

        // visible
        bool visible = settings.value("visible", true).toBool();

        // color
        int color_r = settings.value("color_r", 255).toInt();
        int color_g = settings.value("color_g", 0).toInt();
        int color_b = settings.value("color_b", 0).toInt();
        QColor color = QColor::fromRgb(color_r, color_g, color_b);

        settings.endGroup();

        // update settings
        m_baseBox->setVisible(visible);
        m_baseBox->setColor(color);

        m_trapBox->setVisible(visible);
        m_trapBox->setColor(color);
    }

    // traps
    {
        settings.beginGroup("traps");

        // visible
        bool visible = settings.value("visible", false).toBool();

        // trap color
        int color_r = settings.value("color_r", 255).toInt();
        int color_g = settings.value("color_g", 255).toInt();
        int color_b = settings.value("color_b",   0).toInt();
        QColor color = QColor::fromRgb(color_r, color_g, color_b);

        settings.endGroup();

        // update settings
        setTrapColor(color);
        m_trapBox->showImage(visible);
        initTraps();
    }

    // mesh
    {
        settings.beginGroup("mesh");

        // mode
        Mesh::Mode mode = Mesh::Mode(settings.value("mode", Mesh::Double).toInt());

        // visible
        bool visible = settings.value("visible", false).toBool();

        // trap color
        int acolor_r = settings.value("acolor_r", 255).toInt();
        int acolor_g = settings.value("acolor_g",   0).toInt();
        int acolor_b = settings.value("acolor_b",   0).toInt();
        int acolor_a = settings.value("acolor_a", 255).toInt();
        QColor acolor = QColor::fromRgb(acolor_r, acolor_g, acolor_b, acolor_a);

        int bcolor_r = settings.value("bcolor_r", 255).toInt();
        int bcolor_g = settings.value("bcolor_g", 255).toInt();
        int bcolor_b = settings.value("bcolor_b",   0).toInt();
        int bcolor_a = settings.value("bcolor_a", 255).toInt();
        QColor bcolor = QColor::fromRgb(bcolor_r, bcolor_g, bcolor_b, bcolor_a);

        settings.endGroup();

        // update settings
        m_trapMesh->setVisible(visible);
        m_trapMesh->setColorA(acolor);
        m_trapMesh->setColorB(bcolor);
        m_trapMesh->setMode(mode);
    }

    // electrodes
    {
        settings.beginGroup("electrodes");

        // visible
        bool visible = settings.value("visible", true).toBool();

        // color
        int color_r = settings.value("color_r",   0).toInt();
        int color_g = settings.value("color_g",   0).toInt();
        int color_b = settings.value("color_b",   0).toInt();
        QColor color = QColor::fromRgb(color_r, color_g, color_b);

        settings.endGroup();

        // update settings
        m_lBox->setVisible(visible);
        m_lBox->setColor(color);

        m_rBox->setVisible(visible);
        m_rBox->setColor(color);
    }

    // grid
    {
        settings.beginGroup("grid");

        // visible
        bool visible = settings.value("visible", true).toBool();

        // color
        int color_r = settings.value("color_r", 0).toInt();
        int color_g = settings.value("color_g", 0).toInt();
        int color_b = settings.value("color_b", 0).toInt();
        QColor color = QColor::fromRgb(color_r, color_g, color_b);

        settings.endGroup();

        // update settings
        m_grid->setVisible(visible);
        m_grid->setColor(color);
    }

    // background
    {
        settings.beginGroup("background");

        // color
        int color_r = settings.value("color_r",   0).toInt();
        int color_g = settings.value("color_g", 200).toInt();
        int color_b = settings.value("color_b", 200).toInt();
        QColor color = QColor::fromRgb(color_r, color_g, color_b);

        settings.endGroup();

        // update settings
        setBackgroundColor(color);
    }
}

void LangmuirViewer::setTrapColor(QColor color)
{
    if (color != m_trapColor) {
        m_trapColor = color;
        initTraps();
        emit trapColorChanged(m_trapColor);
    }
}

void LangmuirViewer::setBackgroundColor(QColor color)
{
    QGLViewer::setBackgroundColor(color);
    emit backgroundColorChanged(color);
}

void LangmuirViewer::errorMessage(QString message)
{
    m_error->showMessage(message);
}

void LangmuirViewer::setElectronPointMode(PointCloud::Mode mode)
{
    if (m_electrons != NULL)
    {
        m_electrons->setMode(mode);
    }
}

void LangmuirViewer::setDefectPointMode(PointCloud::Mode mode)
{
    if (m_defects != NULL)
    {
        m_defects->setMode(mode);
    }
}

void LangmuirViewer::setHolePointMode(PointCloud::Mode mode)
{
    if (m_holes != NULL)
    {
        m_holes->setMode(mode);
    }
}

void LangmuirViewer::setPointMode(PointCloud::Mode mode)
{
    setElectronPointMode(mode);
    setDefectPointMode(mode);
    setHolePointMode(mode);
}

void LangmuirViewer::setIterationsPrint(int value)
{
    if (m_world != NULL)
    {
        m_world->parameters().iterationsPrint = value;
        emit showMessage(QString("iterations.print=%1").arg(value));
        emit iterationsPrintChanged(value);
    }
}

void LangmuirViewer::toggleTrapsShown(bool on)
{
    if (m_trapBox != NULL)
    {
        m_trapBox->showImage(on);
        emit showMessage("traps=%d", on);
        emit isShowingTraps(on);
    }
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

void LangmuirViewer::toggleCoulomb(bool on)
{
    if (m_world == NULL) {
        emit showMessage("can not change Coulomb");
        return;
    }

    m_world->parameters().coulombCarriers = on;

    emit isUsingCoulomb(m_world->parameters().coulombCarriers);

    if (on) {
        toggleOpenCL(true);
    }
    else {
        toggleOpenCL(false);
    }
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
    emit isUsingCoulomb(m_world->parameters().coulombCarriers);
    emit currentStepChanged(m_world->parameters().currentStep);
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
    emit currentStepChanged(0);
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
    emit isUsingCoulomb(m_world->parameters().coulombCarriers);
    emit currentStepChanged(m_world->parameters().currentStep);
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
