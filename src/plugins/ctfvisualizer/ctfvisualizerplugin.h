// Copyright (C) 2019 Klarälvdalens Datakonsult AB, a KDAB Group company,
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <extensionsystem/iplugin.h>

namespace CtfVisualizer {
namespace Internal {

class CtfVisualizerPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "CtfVisualizer.json")

public:
    ~CtfVisualizerPlugin();

    bool initialize(const QStringList &arguments, QString *errorString) final;

    class CtfVisualizerPluginPrivate *d = nullptr;
};

} // namespace Internal
} // namespace CtfVisualizer
