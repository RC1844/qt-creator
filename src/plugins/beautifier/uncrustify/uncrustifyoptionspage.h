// Copyright (C) 2016 Lorenz Haas
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <coreplugin/dialogs/ioptionspage.h>

namespace Beautifier::Internal {

class UncrustifySettings;

class UncrustifyOptionsPage final : public Core::IOptionsPage
{
public:
    explicit UncrustifyOptionsPage(UncrustifySettings *settings);
};

} // Beautifier::Internal
