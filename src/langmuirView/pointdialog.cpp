#include "pointdialog.h"
#include "ui_pointdialog.h"
#include "langmuirviewer.h"
#include "pointcloud.h"

#include <QDebug>

PointDialog::PointDialog(LangmuirViewer &viewer, QWidget *parent) :
    QDialog(parent), m_viewer(viewer),
    ui(new Ui::PointDialog)
{
    ui->setupUi(this);
    init();
}

PointDialog::~PointDialog()
{
    delete ui;
}

void PointDialog::init()
{
    update();
    remember();

    connect(&m_viewer.electrons(), SIGNAL(modeChanged(PointCloud::Mode)), this, SLOT(updateComboBoxElectrons(PointCloud::Mode)));
    connect(&m_viewer.defects(), SIGNAL(modeChanged(PointCloud::Mode)), this, SLOT(updateComboBoxDefects(PointCloud::Mode)));
    connect(&m_viewer.holes(), SIGNAL(modeChanged(PointCloud::Mode)), this, SLOT(updateComboBoxHoles(PointCloud::Mode)));

    connect(&m_viewer.electrons(), SIGNAL(pointSizeChanged(float)), this, SLOT(updateSpinBoxElectrons(float)));
    connect(&m_viewer.defects(), SIGNAL(pointSizeChanged(float)), this, SLOT(updateSpinBoxDefects(float)));
    connect(&m_viewer.holes(), SIGNAL(pointSizeChanged(float)), this, SLOT(updateSpinBoxHoles(float)));
}

void PointDialog::update()
{
    updateComboBoxElectrons(m_viewer.electrons().getMode());
    updateComboBoxDefects(m_viewer.defects().getMode());
    updateComboBoxHoles(m_viewer.holes().getMode());

    updateSpinBoxElectrons(m_viewer.electrons().getPointSize());
    updateSpinBoxDefects(m_viewer.defects().getPointSize());
    updateSpinBoxHoles(m_viewer.holes().getPointSize());
}

void PointDialog::updateComboBoxElectrons(PointCloud::Mode mode)
{
    QString string = PointCloud::modeToQString(mode);
    ui->comboBoxElectrons->setCurrentText(string);
}

void PointDialog::updateComboBoxDefects(PointCloud::Mode mode)
{
    QString string = PointCloud::modeToQString(mode);
    ui->comboBoxDefects->setCurrentText(string);
}

void PointDialog::updateComboBoxHoles(PointCloud::Mode mode)
{
    QString string = PointCloud::modeToQString(mode);
    ui->comboBoxHoles->setCurrentText(string);
}

void PointDialog::updateSpinBoxElectrons(float d)
{
    ui->spinBoxElectrons->setValue(d);
}

void PointDialog::updateSpinBoxDefects(float d)
{
    ui->spinBoxDefects->setValue(d);
}

void PointDialog::updateSpinBoxHoles(float d)
{
    ui->spinBoxHoles->setValue(d);
}

void PointDialog::on_comboBoxElectrons_currentTextChanged(const QString & text)
{
    PointCloud::Mode mode = PointCloud::QStringToMode(text);
    m_viewer.electrons().setMode(mode);
}

void PointDialog::on_comboBoxDefects_currentTextChanged(const QString & text)
{
    PointCloud::Mode mode = PointCloud::QStringToMode(text);
    m_viewer.defects().setMode(mode);
}

void PointDialog::on_comboBoxHoles_currentTextChanged(const QString & text)
{
    PointCloud::Mode mode = PointCloud::QStringToMode(text);
    m_viewer.holes().setMode(mode);
}

void PointDialog::on_spinBoxElectrons_valueChanged(double d)
{
    m_viewer.electrons().setPointSize(d);
}

void PointDialog::on_spinBoxDefects_valueChanged(double d)
{
    m_viewer.defects().setPointSize(d);
}

void PointDialog::on_spinBoxHoles_valueChanged(double d)
{
    m_viewer.holes().setPointSize(d);
}

void PointDialog::on_pushButtonReset_clicked()
{
    reset();
}

void PointDialog::on_buttonBox_rejected()
{
    reset();
}

void PointDialog::remember()
{
    e_pointSize_old = m_viewer.electrons().getPointSize();
    d_pointSize_old = m_viewer.defects().getPointSize();
    h_pointSize_old = m_viewer.holes().getPointSize();

    e_mode_old = m_viewer.electrons().getMode();
    d_mode_old = m_viewer.defects().getMode();
    h_mode_old = m_viewer.holes().getMode();
}

void PointDialog::reset()
{
    m_viewer.electrons().setPointSize(e_pointSize_old);
    m_viewer.defects().setPointSize(d_pointSize_old);
    m_viewer.holes().setPointSize(h_pointSize_old);

    m_viewer.electrons().setMode(e_mode_old);
    m_viewer.defects().setMode(d_mode_old);
    m_viewer.holes().setMode(h_mode_old);
}
