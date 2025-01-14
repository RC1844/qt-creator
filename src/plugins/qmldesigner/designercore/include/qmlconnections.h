// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <qmldesignercorelib_global.h>
#include "qmlmodelnodefacade.h"

#include <signalhandlerproperty.h>

namespace QmlDesigner {

class StatesEditorView;

class QMLDESIGNERCORE_EXPORT QmlConnections : public QmlModelNodeFacade
{
    friend StatesEditorView;

public:
    QmlConnections();
    QmlConnections(const ModelNode &modelNode);

    bool isValid() const override;
    static bool isValidQmlConnections(const ModelNode &modelNode);
    void destroy();

    QString target() const;
    void setTarget(const QString &target);

    ModelNode getTargetNode() const;

    QList<SignalHandlerProperty> signalProperties() const;

    static ModelNode createQmlConnections(AbstractView *view);
};

} //QmlDesigner
