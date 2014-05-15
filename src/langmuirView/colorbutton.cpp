#include "colorbutton.h"

QColorDialog * ColorButton::m_colordialog = NULL;

ColorButton::ColorButton(QWidget *parent) :
    QPushButton(parent)
{
    if (m_colordialog == NULL) {
        m_colordialog = new QColorDialog(NULL);
    }

    connect(this, SIGNAL(clicked()), this, SLOT(getColor()));
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
    QString style = QString(
        "QPushButton {"
        "    background-color: rgb(%1, %2, %3)"
        "}"
    ).arg(color.red()).arg(color.green()).arg(color.blue());

    this->setStyleSheet(style);
}

void ColorButton::getColor()
{
    QColor color = m_colordialog->getColor();
    if (color.isValid()) {
        setButtonColor(color);
        emit selectedColor(color);
    }
}
