#include "colorbutton.h"

namespace LangmuirView {

QColorDialog * ColorButton::m_colordialog = NULL;

ColorButton::ColorButton(QWidget *parent) :
    QPushButton(parent)
{
    if (m_colordialog == NULL) {
        m_colordialog = new QColorDialog(NULL);
    }

    connect(this, SIGNAL(clicked()), this, SLOT(getColor()));
    m_color = Qt::white;
}

QColorDialog& ColorButton::colorDialog()
{
    return *m_colordialog;
}

ColorButton::~ColorButton()
{
    if (m_colordialog != NULL) {
        m_colordialog->deleteLater();
        m_colordialog = NULL;
    }
}

void ColorButton::setButtonColor(QColor color)
{
    m_color = color;

    QString style = QString(
        "QPushButton {"
        "    background-color: rgb(%1, %2, %3)"
        "}"
    ).arg(color.red()).arg(color.green()).arg(color.blue());

    this->setStyleSheet(style);
}

void ColorButton::getColor()
{
    QColor color = m_colordialog->getColor(m_color, 0, "Langmuir", QColorDialog::ShowAlphaChannel);
    if (color.isValid()) {
        setButtonColor(color);
        emit selectedColor(color);
    }
}

}
