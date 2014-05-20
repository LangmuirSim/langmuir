#include "isosurfacedialog.h"
#include "ui_isosurfacedialog.h"
#include "langmuirviewer.h"
#include "mesh.h"

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
}

void IsoSurfaceDialog::update()
{
    ui->colorButtonA->setButtonColor(m_viewer.trapMesh().getColorA());
    ui->colorButtonB->setButtonColor(m_viewer.trapMesh().getColorB());
    ui->checkBoxVisible->setChecked(m_viewer.trapMesh().isVisible());
    ui->calculateButton->setEnabled(m_viewer.okToCalculateIsoSurface());
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
