#include "gridview.h"

namespace Langmuir
{

  GridView::GridView(QWidget* parent) : QGraphicsView(parent)
  {
    setWindowFlags(Qt::Dialog | Qt::Tool);

  }

  GridView::GridView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
  {
    setWindowFlags(Qt::Dialog | Qt::Tool);
  }



}

#include "gridview.moc"
