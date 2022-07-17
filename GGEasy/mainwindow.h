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

#include "dockwidget.h"
#include "file.h"
#include "file_plugin.h"
#include "recent.h"

#include <QActionGroup>
#include <QDockWidget>
#include <QMainWindow>
#include <QStack>
#include <QThread>
#include <QTimer>
#include <QTranslator>
#include <qevent.h>

namespace GCode {
class File;
}

class Project;
class QProgressDialog;
class QToolBar;
class Scene;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
    //    friend void FileTree::View::on_doubleClicked(const QModelIndex&);
    friend class Recent;
    friend class Project;

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    // QMainWindow interface
    QMenu* createPopupMenu() override;
    const QDockWidget* dockWidget() const { return dockWidget_; }
    QDockWidget* dockWidget() { return dockWidget_; }

    static void translate(const QString& locale);
    void loadFile(const QString& fileName);
    static void updateTheme();

    //    template <class T>
    //    void createDockWidget() {
    //        if (dynamic_cast<T*>(dockWidget_->widget()))
    //            return;

    //        auto dwContent = new T(dockWidget_);
    //        dwContent->setObjectName(typeid(T).name());

    //        dockWidget_->pop();
    //        dockWidget_->push(dwContent);
    //        dockWidget_->show();
    //    }

signals:
    void parseFile(const QString& filename, int type);

private slots:
    void fileError(const QString& fileName, const QString& error);
    void fileProgress(const QString& fileName, int max, int value);
    void addFileToPro(FileInterface* file);
    void setDockWidget(QWidget* dwContent);

private:
    Ui::MainWindow* ui;
    QDockWidget* dockWidget_ = nullptr;
    Recent recentFiles;
    Recent recentProjects;

    QAction* m_closeAllAct = nullptr;

    QMenu* fileMenu = nullptr;
    QMenu* helpMenu = nullptr;
    QMenu* serviceMenu = nullptr;

    QString lastPath;
    QThread parserThread;

    QToolBar* fileToolBar = nullptr;
    QToolBar* toolpathToolBar = nullptr;
    QToolBar* zoomToolBar = nullptr;

    Project* m_project;
    bool openFlag;

    std::map<int, QAction*> toolpathActions;
    QActionGroup actionGroup;

    QMap<QString, QProgressDialog*> m_progressDialogs;
    QMessageBox reloadQuestion;

    void open();
    bool save();
    bool saveAs();

    void about();
    bool closeProject();

    void initWidgets();

    void printDialog();
    void readSettings();
    void resetToolPathsActions();
    void selectAll();
    void deSelectAll();

    void writeSettings();

    // create actions
    void createActions();
    void createActionsFile();
    void createActionsEdit();
    void createActionsService();
    void createActionsHelp();
    void createActionsZoom();
    void createActionsToolPath();
    void createActionsShape();

    void customContextMenuForToolBar(const QPoint& pos);

    // save GCode
    void saveGCodeFile(int id);
    void saveGCodeFiles();
    void saveSelectedGCodeFiles();

    QString strippedName(const QString& fullFileName);

    void newFile();
    void documentWasModified();
    bool maybeSave();

    void editGcFile(GCode::File* file);

private:
    bool saveFile(const QString& fileName);
    void setCurrentFile(const QString& fileName);

    // QWidget interface
protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void changeEvent(QEvent* event) override;

    // QObject interface
public:
    bool eventFilter(QObject* watched, QEvent* event) override;
};
