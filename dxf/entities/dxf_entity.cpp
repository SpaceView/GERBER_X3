// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "dxf_entity.h"
#include "dxf_file.h"
#include "tables/dxf_layer.h"
#include <QMetaEnum>

namespace Dxf {
Entity::Entity(SectionParser* sp)
    : sp(sp)
{
}

void Entity::draw(const InsertEntity* const i) const
{
    if (i) {
        for (int r = 0; r < i->rowCount; ++r) {
            for (int c = 0; c < i->colCount; ++c) {
                QPointF tr(r * i->rowSpacing, r * i->colSpacing);
                GraphicObject go(toGo());
                i->transform(go, tr);
                i->attachToLayer(std::move(go));
            }
        }
    } else {
        attachToLayer(toGo());
    }
}

void Entity::parse(CodeData& code)
{
    switch (code.code()) {
        //    case LayerName:
        //        layerName = code.string();
        //        break;
        //    case Handle:
        //        handle = code.string();
        //        break;
        //    case ColorNumber:
        //        colorNumber = code;
        //        break;
        //    case SoftPointerID:
        //        softPointerID = code.string();
        //        break;
        //    case NumberOfBytes:
        //        break;

    case EntityName: //  -1
        // qDebug() << __FUNCTION__ << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case EntityType: // 0
        // qDebug() << __FUNCTION__ << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case Handle: // 5
        handle = code.string();
        // qDebug() << __FUNCTION__ << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case SoftPointerID: // 330
        softPointerID = code.string();
        qDebug() << __FUNCTION__ << DataEnum(code.code()) << code;
        break;
    case HardOwnerID: // 360
        // qDebug() << __FUNCTION__ << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case SubclassMarker: // 100
        // qDebug() << __FUNCTION__ << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case E67: // 67
        // qDebug() << __FUNCTION__ << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case E410: // 410
        // qDebug() << __FUNCTION__ << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case LayerName: // 8
        layerName = code.string();
        // qDebug() << __FUNCTION__ << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case LineType: // 6
        // qDebug() << __FUNCTION__ << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case E347: // 347
        // qDebug() << __FUNCTION__ << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case ColorNumber: // 62
        colorNumber = code;
        // qDebug() << __FUNCTION__ << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case LineWeight: // 370
        // qDebug() << __FUNCTION__ << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case LineTypeScale: // 48
        // qDebug() << __FUNCTION__ << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case Visibility: // 60
        // qDebug() << __FUNCTION__ << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case NumberOfBytes: // 92
        // qDebug() << __FUNCTION__ << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case BinaryChunk: // 310
        // qDebug() << __FUNCTION__ << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case A24bitColor: // 420
        // qDebug() << __FUNCTION__ << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case ColorName: // 430
        // qDebug() << __FUNCTION__ << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case TransparencyValue: // 440
        // qDebug() << __FUNCTION__ << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case PlotStyleID: // 390
        // qDebug() << __FUNCTION__ << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    case ShadowMode: // 284
        // qDebug() << __FUNCTION__ << "\n\t" << DataEnum(code.code()) << "\n\t" << code;
        break;
    default:
        qDebug() << __FUNCTION__ << "default" << code;
        break;
    }
}

Entity::Type Entity::toType(const QString& key)
{
    return Type(staticMetaObject.enumerator(0).keyToValue(key.toLocal8Bit().toUpper().data()));
}

QString Entity::typeName(int key)
{
    return staticMetaObject.enumerator(0).valueToKey(key);
}

QString Entity::name() const
{
    return staticMetaObject.enumerator(0).valueToKey(type());
}

QColor Entity::color() const
{
    if (auto layer = sp->file->layer(layerName); layer != nullptr) {
        QColor c(dxfColors[layer->colorNumber()]);
        c.setAlpha(200);
        return c;
    }

    return QColor(255, 0, 255, 100);
}

void Entity::attachToLayer(GraphicObject&& go) const
{
    if (sp == nullptr)
        throw DxfObj::tr("SectionParser is null!");
    else if (sp->file == nullptr)
        throw DxfObj::tr("File in SectionParser is null!");
    else if (sp->file->layer(layerName))
        sp->file->layer(layerName)->addGraphicObject(std::move(go));
    else {
        throw DxfObj::tr("Layer '%1' not found in file!").arg(layerName);
    }
}

Entity::DataEnum Entity::toDataEnum(const QString& key)
{
    return DataEnum(staticMetaObject.enumerator(1).keyToValue(key.toLocal8Bit().toUpper().data()));
}

QPointF polar(QPointF p, float angle, float distance)
{
    // Returns the point at a specified `angle` and `distance` from point `p`.
    return p + QPointF(cos(angle) * distance, sin(angle) * distance);
}

double angle(QPointF p1, QPointF p2)
{
    // Returns angle a line defined by two endpoints and x-axis in radians.
    p2 -= p1;
    return atan2(p2.y(), p2.x());
}

double signedBulgeRadius(QPointF start_point, QPointF end_point, double bulge)
{
    return QLineF(start_point, end_point).length() * (1.0 + (bulge * bulge)) / 4.0 / bulge;
}

std::tuple<QPointF, double, double, double> bulgeToArc(QPointF start_point, QPointF end_point, float bulge)
{
    /*
    Returns arc parameters from bulge parameters.
    Based on Bulge to Arc by `Lee Mac`_.
    Args:
        start_point: start vertex as :class:`Vec2` compatible object
        end_point: end vertex as :class:`Vec2` compatible object
        bulge: bulge value
    Returns:
        Tuple: (center, start_angle, end_angle, radius)
    */
    double r = signedBulgeRadius(start_point, end_point, bulge);
    double a = angle(start_point, end_point) + (M_PI / 2.0 - atan(bulge) * 2.0);
    QPointF c = polar(start_point, a, r);
    if (bulge < 0.0)
        return { c, angle(c, end_point), angle(c, start_point), abs(r) };
    else
        return { c, angle(c, start_point), angle(c, end_point), abs(r) };
}

}
