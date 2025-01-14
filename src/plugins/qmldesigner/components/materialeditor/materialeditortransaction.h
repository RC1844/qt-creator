// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "materialeditorview.h"

namespace QmlDesigner {

class MaterialEditorTransaction : public QObject
{
    Q_OBJECT

public:
    MaterialEditorTransaction(QmlDesigner::MaterialEditorView *materialEditor);

    Q_INVOKABLE void start();
    Q_INVOKABLE void end();

    Q_INVOKABLE bool active() const;

protected:
     void timerEvent(QTimerEvent *event) override;

private:
    QmlDesigner::MaterialEditorView *m_materialEditor = nullptr;
    QmlDesigner::RewriterTransaction m_rewriterTransaction;
    int m_timerId = -1;
};

} // namespace QmlDesigner
