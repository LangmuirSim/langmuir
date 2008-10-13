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

  GridItem::GridItem(int type) : m_type(type)
  {
  }

  QRectF GridItem::boundingRect() const
  {
  }

  QPainterPath GridItem::shape() const
  {
  }

  void GridItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                       QWidget *widget)
  {
  }

}

#include "gridview.moc"
