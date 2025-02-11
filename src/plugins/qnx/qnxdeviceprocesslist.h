// Copyright (C) 2016 BlackBerry Limited. All rights reserved.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <projectexplorer/devicesupport/sshdeviceprocesslist.h>

namespace Qnx::Internal {

class QnxDeviceProcessList : public ProjectExplorer::SshDeviceProcessList
{
public:
    explicit QnxDeviceProcessList(
            const ProjectExplorer::IDeviceConstPtr &device, QObject *parent = nullptr);

private:
    QString listProcessesCommandLine() const override;
    QList<Utils::ProcessInfo> buildProcessList(const QString &listProcessesReply) const override;
};

} // Qnx::Internal
