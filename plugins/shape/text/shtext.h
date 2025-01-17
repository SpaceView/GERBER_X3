/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "gi.h"
#include "plugintypes.h"
#include "shape.h"
#include "shapepluginin.h"
#include <QJsonObject>

class ShTextDialog;

namespace Shapes {

class Text final : public Shape {
    friend ShTextDialog;
    friend Shape;

public:
    explicit Text(QPointF pt1 = {});
    ~Text() = default;

    enum {
        // clang-format off
        BotCenter =   Qt::AlignBottom | Qt::AlignHCenter,
        BotLeft =     Qt::AlignBottom | Qt::AlignLeft,
        BotRight =    Qt::AlignBottom | Qt::AlignRight,

        Center =      Qt::AlignHCenter | Qt::AlignVCenter,
        CenterLeft =  Qt::AlignHCenter | Qt::AlignLeft,
        CenterRight = Qt::AlignHCenter | Qt::AlignRight,

        TopCenter =   Qt::AlignTop | Qt::AlignHCenter,
        TopLeft =     Qt::AlignTop | Qt::AlignLeft,
        TopRight =    Qt::AlignTop | Qt::AlignRight,
        // clang-format on
    };

    struct InternalData {
        friend QDataStream& operator<<(QDataStream& stream, const InternalData& d);
        friend QDataStream& operator>>(QDataStream& stream, InternalData& d);

        QString font {};
        QString text {"Text"};
        Side side {Top};
        double angle {0.0};
        double height {10.0};
        double xy {100.0};
        int handleAlign {BotLeft};
    };

    // QGraphicsItem interface
    int type() const override { return GiType::ShText; }
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    QPainterPath shape() const override; // Shape interface

    // Shape interface
    void redraw() override;
    QString name() const override;
    QIcon icon() const override;
    void setPt(const QPointF& point) override;

    QString text() const;
    void setText(const QString& value);

    Side side() const;
    void setSide(const Side& side);
    // Shape interface
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    void menu(QMenu& menu, FileTree::View* tv) const override;

protected:
    // Shape interface
    void write(QDataStream& stream) const override;
    void read(QDataStream& stream) override;

private:
    InternalData iData;
    InternalData iDataCopy;
    void saveIData();
    InternalData loadIData();

    void save();
    void restore();
    void ok();
};

class PluginImpl : public Shapes::Plugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ShapePlugin_iid FILE "text.json")
    Q_INTERFACES(Shapes::Plugin)

public:
    // ShapePluginTextInterface interface
    int type() const override;
    QIcon icon() const override;
    Shape* createShape(const QPointF& point) const override;
};

} // namespace Shapes
