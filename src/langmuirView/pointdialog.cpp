#include "pointdialog.h"
#include "ui_pointdialog.h"
#include "langmuirviewer.h"
#include "pointcloud.h"

#include <QDebug>

namespace LangmuirView {

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
    connect(&m_viewer.traps(), SIGNAL(modeChanged(PointCloud::Mode)), this, SLOT(updateComboBoxTraps(PointCloud::Mode)));

    connect(&m_viewer.electrons(), SIGNAL(pointSizeChanged(float)), this, SLOT(updateSpinBoxElectrons(float)));
    connect(&m_viewer.defects(), SIGNAL(pointSizeChanged(float)), this, SLOT(updateSpinBoxDefects(float)));
    connect(&m_viewer.holes(), SIGNAL(pointSizeChanged(float)), this, SLOT(updateSpinBoxHoles(float)));
    connect(&m_viewer.traps(), SIGNAL(pointSizeChanged(float)), this, SLOT(updateSpinBoxTraps(float)));

    connect(&m_viewer.electrons(), SIGNAL(visibleChanged(bool)), this, SLOT(updateCheckBoxElectrons(bool)));
    connect(&m_viewer.defects(), SIGNAL(visibleChanged(bool)), this, SLOT(updateCheckBoxDefects(bool)));
    connect(&m_viewer.holes(), SIGNAL(visibleChanged(bool)), this, SLOT(updateCheckBoxHoles(bool)));
    connect(&m_viewer.traps(), SIGNAL(visibleChanged(bool)), this, SLOT(updateCheckBoxTraps(bool)));
}

void PointDialog::update()
{
    updateComboBoxElectrons(m_viewer.electrons().getMode());
    updateComboBoxDefects(m_viewer.defects().getMode());
    updateComboBoxHoles(m_viewer.holes().getMode());
    updateComboBoxTraps(m_viewer.traps().getMode());

    updateSpinBoxElectrons(m_viewer.electrons().getPointSize());
    updateSpinBoxDefects(m_viewer.defects().getPointSize());
    updateSpinBoxHoles(m_viewer.holes().getPointSize());
    updateSpinBoxTraps(m_viewer.traps().getPointSize());

    updateCheckBoxElectrons(m_viewer.electrons().isVisible());
    updateCheckBoxDefects(m_viewer.defects().isVisible());
    updateCheckBoxHoles(m_viewer.holes().isVisible());
    updateCheckBoxTraps(m_viewer.traps().isVisible());
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

void PointDialog::updateComboBoxTraps(PointCloud::Mode mode)
{
    QString string = PointCloud::modeToQString(mode);
    ui->comboBoxTraps->setCurrentText(string);
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

void PointDialog::updateSpinBoxTraps(float d)
{
    ui->spinBoxTraps->setValue(d);
}

void PointDialog::updateCheckBoxElectrons(bool checked)
{
    if (checked) {
        ui->checkBoxElectrons->setCheckState(Qt::Checked);
    } else {
        ui->checkBoxElectrons->setCheckState(Qt::Unchecked);
    }
}

void PointDialog::updateCheckBoxDefects(bool checked)
{
    if (checked) {
        ui->checkBoxDefects->setCheckState(Qt::Checked);
    } else {
        ui->checkBoxDefects->setCheckState(Qt::Unchecked);
    }
}

void PointDialog::updateCheckBoxHoles(bool checked)
{
    if (checked) {
        ui->checkBoxHoles->setCheckState(Qt::Checked);
    } else {
        ui->checkBoxHoles->setCheckState(Qt::Unchecked);
    }
}

void PointDialog::updateCheckBoxTraps(bool checked)
{
    if (checked) {
        ui->checkBoxTraps->setCheckState(Qt::Checked);
    } else {
        ui->checkBoxTraps->setCheckState(Qt::Unchecked);
    }
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

void PointDialog::on_comboBoxTraps_currentTextChanged(const QString & text)
{
    PointCloud::Mode mode = PointCloud::QStringToMode(text);
    m_viewer.traps().setMode(mode);
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

void PointDialog::on_spinBoxTraps_valueChanged(double d)
{
    m_viewer.traps().setPointSize(d);
}

void PointDialog::on_checkBoxElectrons_stateChanged(int state)
{
    switch (state)
    {
        case Qt::Checked:
        {
            m_viewer.electrons().setVisible(true);
            break;
        }
        case Qt::Unchecked: default:
        {
            m_viewer.electrons().setVisible(false);
            break;
        }
    }
}

void PointDialog::on_checkBoxDefects_stateChanged(int state)
{
    switch (state)
    {
        case Qt::Checked:
        {
            m_viewer.defects().setVisible(true);
            break;
        }
        case Qt::Unchecked: default:
        {
            m_viewer.defects().setVisible(false);
            break;
        }
    }
}

void PointDialog::on_checkBoxHoles_stateChanged(int state)
{
    switch (state)
    {
        case Qt::Checked:
        {
            m_viewer.holes().setVisible(true);
            break;
        }
        case Qt::Unchecked: default:
        {
            m_viewer.holes().setVisible(false);
            break;
        }
    }
}

void PointDialog::on_checkBoxTraps_stateChanged(int state)
{
    switch (state)
    {
        case Qt::Checked:
        {
            m_viewer.traps().setVisible(true);
            break;
        }
        case Qt::Unchecked: default:
        {
            m_viewer.traps().setVisible(false);
            break;
        }
    }
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
    t_pointSize_old = m_viewer.traps().getPointSize();

    e_mode_old = m_viewer.electrons().getMode();
    d_mode_old = m_viewer.defects().getMode();
    h_mode_old = m_viewer.holes().getMode();
    t_mode_old = m_viewer.traps().getMode();

    e_visible = m_viewer.electrons().isVisible();
    d_visible = m_viewer.defects().isVisible();
    h_visible = m_viewer.holes().isVisible();
    t_visible = m_viewer.traps().isVisible();
}

void PointDialog::reset()
{
    m_viewer.electrons().setPointSize(e_pointSize_old);
    m_viewer.defects().setPointSize(d_pointSize_old);
    m_viewer.holes().setPointSize(h_pointSize_old);
    m_viewer.traps().setPointSize(t_pointSize_old);

    m_viewer.electrons().setMode(e_mode_old);
    m_viewer.defects().setMode(d_mode_old);
    m_viewer.holes().setMode(h_mode_old);
    m_viewer.traps().setMode(t_mode_old);

    m_viewer.electrons().setVisible(e_visible);
    m_viewer.defects().setVisible(d_visible);
    m_viewer.holes().setVisible(h_visible);
    m_viewer.traps().setVisible(t_visible);
}

}
