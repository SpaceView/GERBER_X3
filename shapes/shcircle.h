#pragma once

#include "shape.h"

namespace Shapes {
class Circle final : public Shape {
public:
    explicit Circle(QPointF center, QPointF pt);
    explicit Circle() { }
    ~Circle();

    // QGraphicsItem interface
    int type() const override { return static_cast<int>(GiType::ShapeC); }
    void redraw() override;
    // Shape interface
    QString name() const override;
    QIcon icon() const override;

    void setPt(const QPointF& pt);
    double radius() const;
    void setRadius(double radius);
    enum {
        Center,
        Point1,
    };

private:
    double m_radius;
};
}
