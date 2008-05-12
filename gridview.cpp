#include "griddisplay.h"

namespace Langmuir
{

GridView::GridView(QWidget* parent = 0) : QGraphicsView(parent)
{
  setWindowFlags(Qt::Dialog | Qt::Tool);

}

GridView::~GridView()
{
}

}

#include "gridview.moc"
