// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "debugger_global.h"

#include <utils/fancymainwindow.h>
#include <utils/statuslabel.h>

#include <QAction>
#include <QPointer>
#include <QToolButton>

#include <functional>

namespace Utils {

// To be used for actions that need hideable toolbuttons.
class DEBUGGER_EXPORT OptionalAction : public QAction
{
    Q_OBJECT

public:
    OptionalAction(const QString &text = QString());
    ~OptionalAction() override;

    void setVisible(bool on);
    void setToolButtonStyle(Qt::ToolButtonStyle style);

public:
    QPointer<QToolButton> m_toolButton;
};

class PerspectiveState
{
public:
    static const char *savesHeaderKey();

    QByteArray mainWindowState;
    QVariantHash headerViewStates;

    friend QDataStream &operator>>(QDataStream &ds, PerspectiveState &state)
        { return ds >> state.mainWindowState >> state.headerViewStates; }
    friend QDataStream &operator<<(QDataStream &ds, const PerspectiveState &state)
        { return ds << state.mainWindowState << state.headerViewStates; }
};

class DEBUGGER_EXPORT Perspective : public QObject
{
public:
    Perspective(const QString &id, const QString &name,
                const QString &parentPerspectiveId = QString(),
                const QString &settingId = QString());
    ~Perspective();

    enum OperationType { SplitVertical, SplitHorizontal, AddToTab, Raise };

    void setCentralWidget(QWidget *centralWidget);
    void addWindow(QWidget *widget,
                   OperationType op,
                   QWidget *anchorWidget,
                   bool visibleByDefault = true,
                   Qt::DockWidgetArea area = Qt::BottomDockWidgetArea);

    void addToolBarAction(QAction *action);
    void addToolBarAction(OptionalAction *action);
    void addToolBarWidget(QWidget *widget);
    void addToolbarSeparator();

    void registerNextPrevShortcuts(QAction *next, QAction *prev);

    void useSubPerspectiveSwitcher(QWidget *widget);

    using ShouldPersistChecker = std::function<bool()>;
    void setShouldPersistChecker(const ShouldPersistChecker &checker);

    QString id() const; // Currently used by GammaRay plugin.
    QString name() const;
    QWidget *centralWidget() const;

    using Callback = std::function<void()>;
    void setAboutToActivateCallback(const Callback &cb);

    void setEnabled(bool enabled);

    void select();
    void destroy();

    static Perspective *findPerspective(const QString &perspectiveId);

    bool isCurrent() const;

private:
    void rampDownAsCurrent();
    void rampUpAsCurrent();

    Perspective(const Perspective &) = delete;
    void operator=(const Perspective &) = delete;

    friend class DebuggerMainWindow;
    friend class DebuggerMainWindowPrivate;
    friend class PerspectivePrivate;
    class PerspectivePrivate *d = nullptr;
};

class DEBUGGER_EXPORT DebuggerMainWindow : public FancyMainWindow
{
    Q_OBJECT

public:
    static DebuggerMainWindow *instance();

    static void ensureMainWindowExists();
    static void doShutdown();

    static void showStatusMessage(const QString &message, int timeoutMS);
    static void enterDebugMode();
    static void leaveDebugMode();

    static QWidget *centralWidgetStack();
    void addSubPerspectiveSwitcher(QWidget *widget);
    static QMenu *perspectiveMenu();

    static Perspective *currentPerspective();

private:
    DebuggerMainWindow();
    ~DebuggerMainWindow() override;

    void savePersistentSettings() const;
    void restorePersistentSettings();

    void contextMenuEvent(QContextMenuEvent *ev) override;

    friend class Perspective;
    friend class PerspectivePrivate;
    friend class DockOperation;
    class DebuggerMainWindowPrivate *d = nullptr;
};

} // Utils

Q_DECLARE_METATYPE(Utils::PerspectiveState)
