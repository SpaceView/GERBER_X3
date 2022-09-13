// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2020                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "sharc.h"
#include "graphicsview.h"
#include "shhandler.h"
#include <QIcon>
#include <QtMath>

namespace Shapes {

Arc::Arc(QPointF center, QPointF pt, QPointF pt2)
    : radius_(QLineF(center, pt).length()) {
    paths_.resize(1);

    handlers.reserve(PtCount);

    handlers.emplace_back(std::make_unique<Handler>(this, Handler::Center));
    handlers.emplace_back(std::make_unique<Handler>(this));
    handlers.emplace_back(std::make_unique<Handler>(this));

    handlers[Center]->setPos(center);
    handlers[Point1]->setPos(pt);
    handlers[Point2]->setPos(pt2);

    redraw();

     App::graphicsView()->scene()->addItem(this);
}

Arc::~Arc() { }

void Arc::redraw() {
    const QLineF l1(handlers[Center]->pos(), handlers[Point1]->pos());
    const QLineF l2(handlers[Center]->pos(), handlers[Point2]->pos());

    radius_ = l1.length();

    const int intSteps = App::settings().clpCircleSegments(radius_);
    const cInt radius = static_cast<cInt>(radius_ * uScale);
    const IntPoint center((handlers[Center]->pos()));
    const double stepAngle = two_pi / intSteps;

    double angle1 = two_pi - qDegreesToRadians(l1.angle());
    double angle2 = two_pi - qDegreesToRadians(l2.angle());

    if (qFuzzyCompare(angle1, two_pi))
        angle1 = 0.0;
    double angle = angle2 - angle1;
    if (angle < 0.0)
        angle = two_pi + angle;

    Path& path = paths_.front();
    path.clear();
    path.reserve(intSteps);

    for (int i = 0; i <= intSteps; i++) {
        const double theta = stepAngle * i;
        if (theta > angle) {
            path.emplace_back(IntPoint(
                static_cast<cInt>(radius * cos(angle2)) + center.X,
                static_cast<cInt>(radius * sin(angle2)) + center.Y));
            break;
        }
        path.emplace_back(IntPoint(
            static_cast<cInt>(radius * cos(angle1 + theta)) + center.X,
            static_cast<cInt>(radius * sin(angle1 + theta)) + center.Y));
    }

    shape_ = QPainterPath();
    shape_.addPolygon(path);
    //    rect_ = shape_.boundingRect();

    setPos({1, 1}); //костыли    //update();
    setPos({0, 0});
}

QString Arc::name() const { return QObject::tr("Arc"); }

QIcon Arc::icon() const { return QIcon::fromTheme("draw-ellipse-arc"); }

void Arc::updateOtherHandlers(Handler* handler) {
    QLineF l(handlers[Center]->pos(), handler->pos());
    radius_ = l.length();

    QLineF l1(handlers[Center]->pos(),
        handlers[Center]->pos() == handlers[Point1]->pos() //если залипло на центр
            ?
            handlers[Center]->pos() + QPointF(1.0, 0.0) :
            handlers[Point1]->pos());
    QLineF l2(handlers[Center]->pos(),
        handlers[Center]->pos() == handlers[Point2]->pos() //если залипло на центр
            ?
            handlers[Center]->pos() + QPointF(1.0, 0.0) :
            handlers[Point2]->pos());

    switch (handlers.indexOf(handler)) {
    case Center:
        break;
    case Point1:
        l2.setLength(radius_);
        handlers[Point2]->QGraphicsItem::setPos(l2.p2());
        break;
    case Point2:
        l1.setLength(radius_);
        handlers[Point1]->QGraphicsItem::setPos(l1.p2());
        break;
    }
}

void Arc::setPt(const QPointF& pt) {
    {
        handlers[Point1]->setPos(pt);
        QLineF l(handlers[Center]->pos(), handlers[Point1]->pos());
        radius_ = l.length();
    }
    {
        QLineF l(handlers[Center]->pos(), handlers[Point2]->pos());
        l.setLength(radius_);
        handlers[Point2]->QGraphicsItem::setPos(l.p2());
    }
    redraw();
}

void Arc::setPt2(const QPointF& pt) {
    QLineF l(handlers[Center]->pos(), pt);
    l.setLength(radius_);

    handlers[Point2]->QGraphicsItem::setPos(l.p2());
    redraw();
}

double Arc::radius() const { return radius_; }

void Arc::setRadius(double radius) {
    if (!qFuzzyCompare(radius_, radius))
        return;
    radius_ = radius;
    redraw();
}

////////////////////////////////////////////////////////////
/// \brief Plugin::Plugin
///
Plugin::Plugin() { }

Plugin::~Plugin() { }

int Plugin::type() const { return GiType::ShCirArc; }

QIcon Plugin::icon() const { return QIcon::fromTheme("draw-ellipse-arc"); }

Shape* Plugin::createShape() { return shape = new Arc(); }

Shape* Plugin::createShape(const QPointF& point) { return shape = new Arc(point, point, point); }

bool Plugin::addShapePoint(const QPointF&) { return ctr++ ? ctr = 0, false : true; }

void Plugin::updateShape(const QPointF& point) {
    if (shape) {
        if (!ctr)
            shape->setPt(point);
        else
            shape->setPt2(point);
    }
}

void Plugin::finalizeShape() {
    if (shape)
        shape->finalize();
    shape = nullptr;
    emit actionUncheck();
}

} // namespace Shapes
