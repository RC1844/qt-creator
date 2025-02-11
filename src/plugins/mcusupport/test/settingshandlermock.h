// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0
#pragma once

#include "settingshandler.h"

#include <gmock/gmock.h>
#include <utils/filepath.h>

namespace McuSupport::Internal {

class SettingsHandlerMock : public SettingsHandler
{
public:
    SettingsHandlerMock() = default;
    ~SettingsHandlerMock() override = default;
    MOCK_METHOD(Utils::FilePath,
                getPath,
                (const QString &, QSettings::Scope, const Utils::FilePath &),
                (const, override));
    MOCK_METHOD(bool,
                write,
                (const QString &, const Utils::FilePath &, const Utils::FilePath &),
                (const, override));
}; //class SettingsHandler
} // namespace McuSupport::Internal
