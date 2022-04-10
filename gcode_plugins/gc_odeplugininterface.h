#pragma once

#include "app.h"
#include <QObject>
#include <QJsonObject>

class QAction;
class QMenu;
class QToolBar;
class QWidget;

class GCodePlugin {
public:
    explicit GCodePlugin() { }
    virtual ~GCodePlugin() { }
    virtual QObject* getObject() = 0;
    //    virtual bool thisIsIt(const QString& fileName) = 0;
    virtual int type() const = 0;
    //    virtual QString folderName() const = 0;

    //    [[nodiscard]] virtual DrillPreviewGiMap createDrillPreviewGi(
    //        [[maybe_unused]] FileInterface* file,
    //        [[maybe_unused]] mvector<Row>& data) { return {}; };
    //    [[nodiscard]] virtual ThermalPreviewGiVec createThermalPreviewGi(
    //        [[maybe_unused]] FileInterface* file,
    //        [[maybe_unused]] const ThParam2& param,
    //        [[maybe_unused]] Tool& tool) { return {}; };
    //    [[nodiscard]] virtual SettingsTabInterface* createSettingsTab([[maybe_unused]] QWidget* parent) { return nullptr; };
    //    [[nodiscard]] virtual FileInterface* createFile() = 0;
    [[nodiscard]] virtual QJsonObject info() const = 0;
    [[nodiscard]] virtual QIcon icon() const = 0;
    [[nodiscard]] virtual QAction* addAction(QMenu* menu, QToolBar* toolbar) = 0;

    //    virtual void addToDrillForm(
    //        [[maybe_unused]] FileInterface* file,
    //        [[maybe_unused]] QComboBox* cbx) {};
    //    virtual void createMainMenu(
    //        [[maybe_unused]] QMenu& menu,
    //        [[maybe_unused]] FileTree::View* tv) {
    //        menu.addAction(QIcon::fromTheme("document-close"), QObject::tr("&Close All Files"), [tv] {
    //            if (QMessageBox::question(tv, "", QObject::tr("Really?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    //                tv->closeFiles();
    //        });
    //    };
    //    virtual void updateFileModel([[maybe_unused]] FileInterface* file) {};

    //    // signals:
    //    virtual void fileError(const QString& fileName, const QString& error) = 0;
    //    virtual void fileWarning([[maybe_unused]] const QString& fileName, [[maybe_unused]] const QString& warning) {};
    //    virtual void fileProgress(const QString& fileName, int max, int value) = 0;
    //    virtual void fileReady(FileInterface* file) = 0;

    //    // slots:
    //    virtual FileInterface* parseFile(const QString& fileName, int type) = 0;
    //signals:
    virtual void setDockWidget(QWidget*) = 0;

protected:
    App app;
    enum { IconSize = 24 };
};

#define GCodeInterface_iid "ru.xray3d.XrSoft.GGEasy.GCodePlugin"

Q_DECLARE_INTERFACE(GCodePlugin, GCodeInterface_iid)
