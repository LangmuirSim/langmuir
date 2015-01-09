#include "isosurfacedialog.h"
#include "ui_isosurfacedialog.h"
#include "langmuirviewer.h"
#include "mesh.h"

namespace LangmuirView {

IsoSurfaceDialog::IsoSurfaceDialog(LangmuirViewer &viewer, QWidget *parent) :
    QDialog(parent), m_viewer(viewer),
    ui(new Ui::IsoSurfaceDialog)
{
    ui->setupUi(this);
    init();
}

IsoSurfaceDialog::~IsoSurfaceDialog()
{
    delete ui;
}

void IsoSurfaceDialog::init()
{
    update();

    connect(&m_viewer.trapMesh(), SIGNAL(colorAChanged(QColor)), ui->colorButtonA, SLOT(setButtonColor(QColor)));
    connect(ui->colorButtonA, SIGNAL(selectedColor(QColor)), &m_viewer.trapMesh(), SLOT(setColorA(QColor)));

    connect(&m_viewer.trapMesh(), SIGNAL(colorBChanged(QColor)), ui->colorButtonB, SLOT(setButtonColor(QColor)));
    connect(ui->colorButtonB, SIGNAL(selectedColor(QColor)), &m_viewer.trapMesh(), SLOT(setColorB(QColor)));

    connect(ui->checkBoxVisible, SIGNAL(toggled(bool)), &m_viewer.trapMesh(), SLOT(setVisible(bool)));
    connect(&m_viewer.trapMesh(), SIGNAL(visibleChanged(bool)), ui->checkBoxVisible, SLOT(setChecked(bool)));

    connect(&m_viewer, SIGNAL(canCalculateIsoSurface(bool)), this, SLOT(setCalculateEnabled(bool)));
    connect(&m_viewer, SIGNAL(isoSurfaceProgress(int)), ui->calculateProgress, SLOT(setValue(int)));

    connect(&m_viewer.trapMesh(), SIGNAL(modeChanged(Mesh::Mode)), this, SLOT(updateComboBoxMode(Mesh::Mode)));

    connect(&m_viewer.trapMesh(), SIGNAL(colorAChanged(QColor)), this, SLOT(extractAlpha(QColor)));
    connect(&m_viewer.trapMesh(), SIGNAL(colorBChanged(QColor)), this, SLOT(extractAlpha(QColor)));
    connect(this, SIGNAL(sendAlpha(int)), this, SLOT(updateSpinBoxAlpha(int)));
}

void IsoSurfaceDialog::update()
{
    ui->colorButtonA->setButtonColor(m_viewer.trapMesh().getColorA());
    ui->colorButtonB->setButtonColor(m_viewer.trapMesh().getColorB());
    ui->checkBoxVisible->setChecked(m_viewer.trapMesh().isVisible());
    ui->calculateButton->setEnabled(m_viewer.okToCalculateIsoSurface());

    QColor colorA = m_viewer.trapMesh().getColorA();
    updateSpinBoxAlpha(colorA.alpha());

    updateComboBoxMode(m_viewer.trapMesh().getMode());
}

void IsoSurfaceDialog::on_calculateButton_clicked()
{
    ui->calculateProgress->setValue(0);
    ui->checkBoxVisible->setChecked(false);
    m_viewer.generateIsoSurface(ui->doubleSpinBoxIsoValue->value());
}

void IsoSurfaceDialog::setCalculateEnabled(bool enabled)
{
    ui->calculateButton->setEnabled(enabled);
}

void IsoSurfaceDialog::setProgressRange(int low, int high)
{
    ui->calculateProgress->setRange(low, high);
}

void IsoSurfaceDialog::setProgress(int value)
{
    ui->calculateProgress->setValue(value);
}

void IsoSurfaceDialog::updateComboBoxMode(Mesh::Mode mode)
{
    QString string = Mesh::modeToQString(mode);
    ui->comboBoxMode->setCurrentText(string);
}

void IsoSurfaceDialog::updateSpinBoxAlpha(int value)
{
    ui->spinBoxAlpha->setValue(value);
}

void IsoSurfaceDialog::on_comboBoxMode_currentTextChanged(const QString & text)
{
    Mesh::Mode mode = Mesh::QStringToMode(text);
    m_viewer.trapMesh().setMode(mode);
}

void IsoSurfaceDialog::on_spinBoxAlpha_valueChanged(int value)
{
    QColor colorA  = m_viewer.trapMesh().getColorA();
    QColor colorB  = m_viewer.trapMesh().getColorB();

    colorA.setAlpha(value);
    colorB.setAlpha(value);

    m_viewer.trapMesh().setColorA(colorA);
    m_viewer.trapMesh().setColorB(colorB);
}

void IsoSurfaceDialog::extractAlpha(QColor color)
{
    emit sendAlpha(color.alpha());
}

}
