// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <projectexplorer/devicesupport/desktopdevice.h>

namespace McuSupport {
namespace Internal {

class McuSupportDevice final : public ProjectExplorer::DesktopDevice
{
    Q_DECLARE_TR_FUNCTIONS(McuSupport::Internal::McuSupportDevice)

public:
    static ProjectExplorer::IDevice::Ptr create();

private:
    McuSupportDevice();
};

class McuSupportDeviceFactory final : public ProjectExplorer::IDeviceFactory
{
public:
    McuSupportDeviceFactory();
};

} // namespace Internal
} // namespace McuSupport
