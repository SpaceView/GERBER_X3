#pragma once

#include "shape.h"

namespace Shapes {
class Rectangle final : public Shape {
public:
    explicit Rectangle(QPointF pt1, QPointF pt2);
    Rectangle(QDataStream& stream);
    ~Rectangle();

    // QGraphicsItem interface
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override { return GiShapeR; }

    // GraphicsItem interface
    void redraw() override;
    QString name() const override { return QObject::tr("Rectangle"); }
    void setPt(const QPointF& pt);
    enum {
        Center,
        Point1,
        Point2,
    };
};
}
