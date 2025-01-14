// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <utils/fileutils.h>

#include <QHash>

namespace Utils { class Environment; }

namespace Autotest {

class ITestFramework;

namespace Internal {

struct TestCase
{
    QString name;
    bool multipleTestCases;
};
using TestCases = QList<TestCase>;

namespace QTestUtils {

bool isQTestMacro(const QByteArray &macro);
QHash<Utils::FilePath, TestCases> testCaseNamesForFiles(ITestFramework *framework,
                                                        const Utils::FilePaths &files);
QMultiHash<Utils::FilePath, Utils::FilePath> alternativeFiles(ITestFramework *framework,
                                                              const Utils::FilePaths &files);
QStringList filterInterfering(const QStringList &provided, QStringList *omitted, bool isQuickTest);
Utils::Environment prepareBasicEnvironment(const Utils::Environment &env);

} // namespace QTestUtils
} // namespace Internal
} // namespace Autotest
