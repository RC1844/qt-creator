/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#pragma once

#include <QElapsedTimer>
#include <QPointer>
#include <QQmlPropertyMap>
#include <QQuickWidget>

QT_BEGIN_NAMESPACE
class QShortcut;
QT_END_NAMESPACE

namespace QmlDesigner {

class NodeInstanceView;

namespace Experimental {

class StatesEditorModel;
class StatesEditorView;

namespace Internal { class StatesEditorImageProvider; }

class StatesEditorWidget : public QQuickWidget
{
    Q_OBJECT

public:
    StatesEditorWidget(StatesEditorView *m_statesEditorView, StatesEditorModel *statesEditorModel);
    ~StatesEditorWidget() override;

    int currentStateInternalId() const;
    void setCurrentStateInternalId(int internalId);
    void setNodeInstanceView(NodeInstanceView *nodeInstanceView);

    void showAddNewStatesButton(bool showAddNewStatesButton);

    static QString qmlSourcesPath();

protected:
    void showEvent(QShowEvent *) override;
    void focusOutEvent(QFocusEvent *focusEvent) override;
    void focusInEvent(QFocusEvent *focusEvent) override;

private:
    void reloadQmlSource();

private:
    QPointer<StatesEditorView> m_statesEditorView;
    Internal::StatesEditorImageProvider *m_imageProvider;
    QShortcut *m_qmlSourceUpdateShortcut;
    QElapsedTimer m_usageTimer;
};

} // namespace Experimental
} // namespace QmlDesigner
