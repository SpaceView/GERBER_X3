/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ***********************************************************8********************/
#pragma once
#ifdef GBR_
#include "ex_types.h"
#include "gbr_types.h"
#endif

#include "gc_plugin.h"
#include "gcode.h"

#include <QHeaderView>
#include <QMenu>
#include <QToolBar>
#include <QWidget>

using HV = std::variant<const QPolygonF*, const QPointF>;
using HK = std::tuple<int, double, bool>;
using HoleMap = std::map<HK, mvector<HV>>;

namespace Ui {
class DrillForm;
}

class AbstractDrillPrGI;
class DrillModel;
class Header;
class QCheckBox;

class DrillForm : public QWidget {
    Q_OBJECT

public:
    explicit DrillForm(QWidget* parent = nullptr);
    ~DrillForm() override;

#ifdef GBR_
    void setExcellonTools(const Excellon::Tools& value);
#endif
    void updateFiles();
    static bool canToShow();

public slots:
    void on_pbClose_clicked();

private slots:
    void on_pbCreate_clicked();

private:
    GCode::GCodeType worckType_ = GCode::Drill;
    GCode::SideOfMilling side_ = GCode::Inner;

    void on_cbxFileCurrentIndexChanged(int index);
    void on_clicked(const QModelIndex& index);
    void on_doubleClicked(const QModelIndex& current);
    void on_currentChanged(const QModelIndex& current, const QModelIndex& previous);
    void on_customContextMenuRequested(const QPoint& pos);

    void updateToolsOnGi(int apToolId);
    void pickUpTool();

    //    inline void updateCreateButton();
    inline void setSelected(int id, bool fl);
    inline void zoonToSelected();
    inline void deselectAll();
    DrillModel* model = nullptr;
    Ui::DrillForm* ui;

    int type_;
#ifdef GBR_
    Gerber::ApertureMap m_apertures;
    Excellon::Tools m_tools;
#endif
    HoleMap peview_;
    std::map<int, mvector<std::unique_ptr<AbstractDrillPrGI>>> giPeview_;
    FileInterface* file = nullptr;
    QCheckBox* checkBox;
    Header* header;
    void clear();
};

class Header : public QHeaderView {
    Q_OBJECT

public:
    Header(Qt::Orientation orientation, QWidget* parent = nullptr);
    ~Header() override;

    enum {
        XOffset = 5,
        DelegateSize = 16
    };

    void setAll(bool ch);
    void togle(int index);
    void set(int index, bool ch);
    static QRect getRect(const QRect& rect);

signals:
    void onChecked(int = -1);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;

private:
    int flag = Qt::Unchecked;
    mutable mvector<QRect> m_checkRect;
    void setChecked(int index, bool ch);
    bool checked(int index) const;
    DrillModel* model() const;
};

class GCPluginImpl : public GCodePlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID GCodeInterface_iid FILE "drill.json")
    Q_INTERFACES(GCodePlugin)

    // GCodePlugin interface
public:
    QIcon icon() const override { return QIcon::fromTheme("drill-path"); }
    QKeySequence keySequence() const override { return { "Ctrl+Shift+D" }; }
    QWidget* createForm() override { return new DrillForm; };
    bool canToShow() const override { return DrillForm::canToShow(); }
    int type() const override { return GCode::Drill; }
};

#include "app.h"
