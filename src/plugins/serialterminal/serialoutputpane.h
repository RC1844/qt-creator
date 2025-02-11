// Copyright (C) 2018 Benjamin Balga
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "serialdevicemodel.h"
#include "serialterminalsettings.h"

#include <coreplugin/ioutputpane.h>
#include <utils/outputformat.h>

#include <QWidget>

#include <memory>

QT_BEGIN_NAMESPACE
class QAbstractButton;
class QButtonGroup;
class QComboBox;
class QToolButton;
QT_END_NAMESPACE

namespace Core { class OutputWindow; }

namespace SerialTerminal {
namespace Internal {

class ComboBox;
class ConsoleLineEdit;
class SerialControl;
class TabWidget;

class SerialOutputPane : public Core::IOutputPane
{
    Q_OBJECT

public:
    enum CloseTabMode {
        CloseTabNoPrompt,
        CloseTabWithPrompt
    };

    enum BehaviorOnOutput {
        Flash,
        Popup
    };

    explicit SerialOutputPane(Settings &settings);

    // IOutputPane
    QWidget *outputWidget(QWidget *parent) final;
    QList<QWidget *> toolBarWidgets() const final;
    QString displayName() const final;

    int priorityInStatusBar() const final;
    void clearContents() final;
    bool canFocus() const final;
    bool hasFocus() const final;
    void setFocus() final;

    bool canNext() const final;
    bool canPrevious() const final;
    void goToNext() final;
    void goToPrev() final;
    bool canNavigate() const final;

    void createNewOutputWindow(SerialControl *rc);

    bool closeTabs(CloseTabMode mode);

    void appendMessage(SerialControl *rc, const QString &out, Utils::OutputFormat format);

    void setSettings(const Settings &settings);

signals:
    void settingsChanged(const Settings &settings);

private:
    class SerialControlTab {
    public:
        explicit SerialControlTab(SerialControl *serialControl = nullptr,
                                  Core::OutputWindow *window = nullptr);
        SerialControl *serialControl = nullptr;
        Core::OutputWindow *window = nullptr;
        BehaviorOnOutput behaviorOnOutput = Flash;
        int inputCursorPosition = 0;
        QString inputText;
        QByteArray lineEnd;
        int lineEndingIndex = 0;
    };

    void createToolButtons();
    void updateLineEndingsComboBox();
    void updatePortsList();

    void contextMenuRequested(const QPoint &pos, int index);

    void enableDefaultButtons();
    void enableButtons(const SerialControl *rc, bool isRunning);
    void tabChanged(int i);

    bool isRunning() const;

    void activePortNameChanged(int index);
    void activeBaudRateChanged(int index);
    void defaultLineEndingChanged(int index);

    void connectControl();
    void disconnectControl();
    void resetControl();
    void openNewTerminalControl();
    void sendInput();

    bool closeTab(int index, CloseTabMode cm = CloseTabWithPrompt);
    int indexOf(const SerialControl *rc) const;
    int indexOf(const QWidget *outputWindow) const;
    int currentIndex() const;
    SerialControl *currentSerialControl() const;
    bool isCurrent(const SerialControl *rc) const;
    int findTabWithPort(const QString &portName) const;
    int findRunningTabWithPort(const QString &portName) const;
    void handleOldOutput(Core::OutputWindow *window) const;

    void updateCloseActions();

    std::unique_ptr<QWidget> m_mainWidget;
    ConsoleLineEdit *m_inputLine = nullptr;
    QComboBox *m_lineEndingsSelection = nullptr;
    TabWidget *m_tabWidget = nullptr;
    Settings m_settings;
    QVector<SerialControlTab> m_serialControlTabs;
    int m_prevTabIndex = -1;

    SerialDeviceModel *m_devicesModel = nullptr;

    QAction *m_closeCurrentTabAction = nullptr;
    QAction *m_closeAllTabsAction = nullptr;
    QAction *m_closeOtherTabsAction = nullptr;

    QAction *m_disconnectAction = nullptr;
    QToolButton *m_connectButton = nullptr;
    QToolButton *m_disconnectButton = nullptr;
    QToolButton *m_resetButton = nullptr;
    QToolButton *m_newButton = nullptr;
    ComboBox *m_portsSelection = nullptr;
    ComboBox *m_baudRateSelection = nullptr;

    QString m_currentPortName;
    float m_zoom = 1.0;
};

} // namespace Internal
} // namespace SerialTerminal
