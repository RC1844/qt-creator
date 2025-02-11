// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <QUrl>
#include <QString>
#include <QPointer>

namespace ProjectExplorer {
class Target;
}

namespace QmlPreview {
namespace Internal {

class QmlPreviewFileOnTargetFinder
{
public:
    void setTarget(ProjectExplorer::Target *target);
    ProjectExplorer::Target *target() const;

    QString findPath(const QString &filePath, bool *success = nullptr) const;
    QUrl findUrl(const QString &filePath, bool *success = nullptr) const;

private:
    QPointer<ProjectExplorer::Target> m_target;
};

} // namespace Internal
} // namespace QmlPreview
