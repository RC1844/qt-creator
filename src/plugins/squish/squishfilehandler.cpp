// Copyright (C) 2022 The Qt Company Ltd
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "squishfilehandler.h"

#include "opensquishsuitesdialog.h"
#include "squishconstants.h"
#include "squishsettings.h"
#include "squishtesttreemodel.h"
#include "squishtools.h"
#include "suiteconf.h"
#include "squishtr.h"

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/icore.h>
#include <projectexplorer/session.h>
#include <utils/algorithm.h>
#include <utils/aspects.h>
#include <utils/layoutbuilder.h>
#include <utils/qtcassert.h>

#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace Squish {
namespace Internal {

static const char SK_OpenSuites[] = "SquishOpenSuites";

static SquishFileHandler *m_instance = nullptr;

class MappedAutDialog : public QDialog
{
public:
    MappedAutDialog()
    {
        auto label = new QLabel(Tr::tr("Application:"), this);
        aut.addItem(Tr::tr("<No Application>"));
        arguments.setLabelText(Tr::tr("Arguments:"));
        arguments.setDisplayStyle(Utils::StringAspect::LineEditDisplay);

        QWidget *widget = new QWidget(this);
        auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

        using namespace Utils::Layouting;
        Form {
            label, &aut, br,
            arguments,
            st
        }.attachTo(widget);

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(widget);
        layout->addWidget(buttons);
        setLayout(layout);

        QPushButton *okButton = buttons->button(QDialogButtonBox::Ok);
        okButton->setEnabled(false);
        connect(okButton, &QPushButton::clicked,
                this, &QDialog::accept);
        connect(buttons->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
                this, &QDialog::reject);
        connect(&aut, &QComboBox::currentIndexChanged,
                this, [this, okButton] (int index) {
            okButton->setEnabled(index > 0);
        });
        setWindowTitle(Tr::tr("Recording Settings"));

        auto squishTools = SquishTools::instance();
        connect(squishTools, &SquishTools::queryFinished, this,
                [this] (const QString &out) {
            SquishServerSettings s;
            s.setFromXmlOutput(out);
            QApplication::restoreOverrideCursor();
            for (const QString &app : s.mappedAuts.keys())
                aut.addItem(app);
        });

        QApplication::setOverrideCursor(Qt::WaitCursor);
        squishTools->queryServerSettings();
    }


    QComboBox aut;
    Utils::StringAspect arguments;
};

SquishFileHandler::SquishFileHandler(QObject *parent)
    : QObject(parent)
{
    m_instance = this;
    auto sessionManager = ProjectExplorer::SessionManager::instance();
    connect(sessionManager, &ProjectExplorer::SessionManager::sessionLoaded,
            this, &SquishFileHandler::onSessionLoaded);
}

SquishFileHandler *SquishFileHandler::instance()
{
    if (!m_instance)
        m_instance = new SquishFileHandler;

    return m_instance;
}

enum class SharedType { SharedFoldersAndFiles, SharedData };

static SquishTestTreeItem::Type itemTypeForSharedType(SharedType sharedType, bool isDirectory)
{
    switch (sharedType) {
    case SharedType::SharedFoldersAndFiles:
        return isDirectory ? SquishTestTreeItem::SquishSharedFolder
                           : SquishTestTreeItem::SquishSharedFile;
    case SharedType::SharedData:
        return isDirectory ? SquishTestTreeItem::SquishSharedDataFolder
                           : SquishTestTreeItem::SquishSharedData;
    }
    QTC_CHECK(false);
    return SquishTestTreeItem::SquishSharedFile;
}

static bool isIgnoredSuffix(const QString &suffix)
{
    const QStringList ignored{ "pyc", "swp", "bak", "autosave"};
    return suffix.endsWith('~') || ignored.contains(suffix);
}

static void addAllEntriesRecursively(SquishTestTreeItem *item, SharedType sharedType)
{
    const Utils::FilePath folder = Utils::FilePath::fromString(item->filePath());

    for (const Utils::FilePath &file : folder.dirEntries(QDir::AllEntries | QDir::NoDotAndDotDot)) {
        const bool isDirectory = file.isDir();
        if (!file.isFile() && !isDirectory)
            continue;

        if (isDirectory && file.fileName() == "__pycache__")
            continue;

        if (!isDirectory && isIgnoredSuffix(file.suffix()))
            continue;

        SquishTestTreeItem *child = new SquishTestTreeItem(
                    file.fileName(), itemTypeForSharedType(sharedType, isDirectory));
        child->setFilePath(file.toString());
        if (isDirectory)
            addAllEntriesRecursively(child, sharedType);

        item->appendChild(child);
    }
}

static void processSharedSubFolders(SquishTestTreeItem *item, const Utils::FilePath &directory,
                                    SharedType sharedType)
{
    SquishTestTreeItem *sharedItem = new SquishTestTreeItem(directory.fileName(),
                                                            SquishTestTreeItem::SquishSharedRoot);
    sharedItem->setFilePath(directory.path());
    addAllEntriesRecursively(sharedItem, sharedType);
    if (sharedItem->hasChildren())
        item->appendChild(sharedItem);
    else
        delete sharedItem;
}

SquishTestTreeItem *createSuiteTreeItem(const QString &name,
                                        const QString &filePath,
                                        const QStringList &cases)
{
    SquishTestTreeItem *item = new SquishTestTreeItem(name, SquishTestTreeItem::SquishSuite);
    item->setFilePath(filePath);
    for (const QString &testCase : cases) {
        const Utils::FilePath testCaseFP = Utils::FilePath::fromString(testCase);
        const Utils::FilePath testCaseDir = testCaseFP.parentDir();
        SquishTestTreeItem *child = new SquishTestTreeItem(testCaseDir.fileName(),
                                                           SquishTestTreeItem::SquishTestCase);
        child->setFilePath(testCase);
        item->appendChild(child);

        if (const Utils::FilePath data = testCaseDir.pathAppended("testdata"); data.isDir())
            processSharedSubFolders(child, data, SharedType::SharedData);

        for (auto &file : testCaseDir.dirEntries(QDir::AllEntries | QDir::NoDotAndDotDot)) {
            // ignore current test script and testdata folder
            const bool isDir = file.isDir();
            if (file == testCaseFP || (isDir && file.fileName() == "testdata"))
                continue;

            SquishTestTreeItem *other
                = new SquishTestTreeItem(file.fileName(),
                                         isDir ? SquishTestTreeItem::SquishSharedFolder
                                               : SquishTestTreeItem::SquishSharedFile);
            other->setFilePath(file.toString());
            if (isDir)
                addAllEntriesRecursively(other, SharedType::SharedFoldersAndFiles);

            child->appendChild(other);
        }
    }

    const Utils::FilePath baseDir = Utils::FilePath::fromString(filePath).absolutePath();
    if (const Utils::FilePath scripts = baseDir.pathAppended("shared/scripts"); scripts.isDir())
        processSharedSubFolders(item, scripts, SharedType::SharedFoldersAndFiles);
    if (const Utils::FilePath data = baseDir.pathAppended("shared/testdata"); data.isDir())
        processSharedSubFolders(item, data, SharedType::SharedData);

    return item;
}

void SquishFileHandler::modifySuiteItem(const QString &suiteName,
                                        const QString &filePath,
                                        const QStringList &cases)
{
    SquishTestTreeItem *item = createSuiteTreeItem(suiteName, filePath, cases);
    // TODO update file watcher
    m_suites.insert(suiteName, filePath);
    emit suiteTreeItemModified(item, suiteName);
}

void SquishFileHandler::openTestSuites()
{
    OpenSquishSuitesDialog dialog;
    dialog.exec();
    QMessageBox::StandardButton replaceSuite = QMessageBox::NoButton;
    const QStringList chosenSuites = dialog.chosenSuites();
    for (const QString &suite : chosenSuites) {
        const QDir suiteDir(suite);
        const QString suiteName = suiteDir.dirName();
        const QStringList cases = SuiteConf::validTestCases(suite);
        const QFileInfo suiteConf(suiteDir, "suite.conf");

        if (m_suites.contains(suiteName)) {
            if (replaceSuite == QMessageBox::YesToAll) {
                modifySuiteItem(suiteName, suiteConf.absoluteFilePath(), cases);
            } else if (replaceSuite != QMessageBox::NoToAll) {
                replaceSuite
                    = QMessageBox::question(Core::ICore::dialogParent(),
                                            Tr::tr("Suite Already Open"),
                                            Tr::tr("A test suite with the name \"%1\" is already open."
                                                   "\nClose the opened test suite and replace it "
                                                   "with the new one?")
                                                .arg(suiteName),
                                            QMessageBox::Yes | QMessageBox::YesToAll
                                                | QMessageBox::No | QMessageBox::NoToAll,
                                            QMessageBox::No);
                if (replaceSuite == QMessageBox::YesToAll || replaceSuite == QMessageBox::Yes)
                    modifySuiteItem(suiteName, suiteConf.absoluteFilePath(), cases);
            }
        } else {
            SquishTestTreeItem *item = createSuiteTreeItem(suiteName,
                                                           suiteConf.absoluteFilePath(),
                                                           cases);
            // TODO add file watcher
            m_suites.insert(suiteName, suiteConf.absoluteFilePath());
            emit testTreeItemCreated(item);
        }
    }
    emit suitesOpened();
    ProjectExplorer::SessionManager::setValue(SK_OpenSuites, m_suites.values());
}

void SquishFileHandler::openTestSuite(const Utils::FilePath &suitePath, bool isReopen)
{
    const QString suiteName = suitePath.parentDir().fileName();
    const QString suitePathStr = suitePath.toString();
    const QStringList cases = SuiteConf::validTestCases(suitePath.parentDir().toString());

    if (m_suites.contains(suiteName)) {
        if (isReopen) {
            modifySuiteItem(suiteName, suitePathStr, cases);
            return;
        }
        QMessageBox::Button replaceSuite
                = QMessageBox::question(Core::ICore::dialogParent(),
                                        Tr::tr("Suite Already Open"),
                                        Tr::tr("A test suite with the name \"%1\" is already open."
                                               "\nClose the opened test suite and replace it "
                                               "with the new one?")
                                        .arg(suiteName),
                                        QMessageBox::Yes | QMessageBox::No,
                                        QMessageBox::No);
        if (replaceSuite == QMessageBox::Yes)
            modifySuiteItem(suiteName, suitePathStr, cases);
    } else {
        SquishTestTreeItem *item = createSuiteTreeItem(suiteName, suitePathStr, cases);
        // TODO add file watcher
        m_suites.insert(suiteName, suitePathStr);
        emit testTreeItemCreated(item);
    }
    ProjectExplorer::SessionManager::setValue(SK_OpenSuites, m_suites.values());
}

void SquishFileHandler::closeTestSuite(const QString &suiteName)
{
    if (!m_suites.contains(suiteName))
        return;

    // TODO close respective editors if there are any
    // TODO remove file watcher
    m_suites.remove(suiteName);
    emit suiteTreeItemRemoved(suiteName);
    ProjectExplorer::SessionManager::setValue(SK_OpenSuites, m_suites.values());
}

void SquishFileHandler::closeAllTestSuites()
{
    closeAllInternal();
    ProjectExplorer::SessionManager::setValue(SK_OpenSuites, m_suites.values());
}

void SquishFileHandler::closeAllInternal()
{
    // TODO close respective editors if there are any
    // TODO remove file watcher
    const QStringList &suiteNames = m_suites.keys();
    m_suites.clear();
    for (const QString &suiteName : suiteNames)
        emit suiteTreeItemRemoved(suiteName);
}

void SquishFileHandler::runTestCase(const QString &suiteName, const QString &testCaseName)
{
    QTC_ASSERT(!suiteName.isEmpty() && !testCaseName.isEmpty(), return );

    if (SquishTools::instance()->state() != SquishTools::Idle)
        return;

    const QDir suitePath = QFileInfo(m_suites.value(suiteName)).absoluteDir();
    if (!suitePath.exists() || !suitePath.isReadable()) {
        QMessageBox::critical(Core::ICore::dialogParent(),
                              Tr::tr("Test Suite Path Not Accessible"),
                              Tr::tr("The path \"%1\" does not exist or is not accessible.\n"
                                     "Refusing to run test case \"%2\".")
                                  .arg(QDir::toNativeSeparators(suitePath.absolutePath()))
                                  .arg(testCaseName));
        return;
    }

    SquishTools::instance()->runTestCases(suitePath.absolutePath(), QStringList(testCaseName));
}

void SquishFileHandler::runTestSuite(const QString &suiteName)
{
    QTC_ASSERT(!suiteName.isEmpty(), return );

    if (SquishTools::instance()->state() != SquishTools::Idle)
        return;

    const QString suiteConf = m_suites.value(suiteName);
    const QDir suitePath = QFileInfo(suiteConf).absoluteDir();
    if (!suitePath.exists() || !suitePath.isReadable()) {
        QMessageBox::critical(Core::ICore::dialogParent(),
                              Tr::tr("Test Suite Path Not Accessible"),
                              Tr::tr("The path \"%1\" does not exist or is not accessible.\n"
                                     "Refusing to run test cases.")
                                  .arg(QDir::toNativeSeparators(suitePath.absolutePath())));
        return;
    }

    QStringList testCases = SquishTestTreeModel::instance()->getSelectedSquishTestCases(suiteConf);
    if (testCases.isEmpty()) {
        QMessageBox::information(Core::ICore::dialogParent(),
                                 Tr::tr("No Test Cases Defined"),
                                 Tr::tr("Test suite \"%1\" does not contain any test cases.")
                                 .arg(suiteName));
        return;
    }
    SquishTools::instance()->runTestCases(suitePath.absolutePath(), testCases);
}

void SquishFileHandler::recordTestCase(const QString &suiteName, const QString &testCaseName)
{
    QTC_ASSERT(!suiteName.isEmpty() && !testCaseName.isEmpty(), return );

    if (SquishTools::instance()->state() != SquishTools::Idle)
        return;

    const QDir suitePath = QFileInfo(m_suites.value(suiteName)).absoluteDir();
    if (!suitePath.exists() || !suitePath.isReadable()) {
        QMessageBox::critical(Core::ICore::dialogParent(),
                              Tr::tr("Test Suite Path Not Accessible"),
                              Tr::tr("The path \"%1\" does not exist or is not accessible.\n"
                                     "Refusing to record test case \"%2\".")
                                  .arg(QDir::toNativeSeparators(suitePath.absolutePath()))
                                  .arg(testCaseName));
        return;
    }

    SuiteConf conf = SuiteConf::readSuiteConf(
                Utils::FilePath::fromString(m_suites.value(suiteName)));
    if (conf.aut().isEmpty()) {
        MappedAutDialog dialog;

        if (dialog.exec() != QDialog::Accepted)
            return;

        conf.setAut(dialog.aut.currentText());
        conf.setArguments(dialog.arguments.value());
    }

    SquishTools::instance()->recordTestCase(suitePath.absolutePath(), testCaseName, conf);
}

void addAllEntriesRecursively(SquishTestTreeItem *item)
{
    QDir folder(item->filePath());

    const QFileInfoList entries = folder.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for (const QFileInfo &info : entries) {
        const QString &path = info.absoluteFilePath();
        // TODO improve this later? Squish refuses directories containing Squish test suites
        const bool isDir = info.isDir();
        if (!info.isFile() && !isDir)
            continue;

        SquishTestTreeItem *child
            = new SquishTestTreeItem(info.fileName(),
                                     isDir ? SquishTestTreeItem::SquishSharedFolder
                                           : SquishTestTreeItem::SquishSharedFile);
        child->setFilePath(path);

        if (info.isDir())
            addAllEntriesRecursively(child);

        item->appendChild(child);
    }
}

void SquishFileHandler::addSharedFolder()
{
    const QString &chosen = QFileDialog::getExistingDirectory(Core::ICore::dialogParent(),
                                                              Tr::tr("Select Global Script Folder"));
    if (chosen.isEmpty())
        return;

    if (m_sharedFolders.contains(chosen))
        return;

    m_sharedFolders.append(chosen);
    SquishTestTreeItem *item = new SquishTestTreeItem(chosen,
                                                      SquishTestTreeItem::SquishSharedFolder);
    item->setFilePath(chosen);
    addAllEntriesRecursively(item);
    emit testTreeItemCreated(item);
}

bool SquishFileHandler::removeSharedFolder(const QString &folder)
{
    if (m_sharedFolders.contains(folder))
        return m_sharedFolders.removeOne(folder);

    return false;
}

void SquishFileHandler::removeAllSharedFolders()
{
    m_sharedFolders.clear();
}

void SquishFileHandler::openObjectsMap(const QString &suiteName)
{
    QTC_ASSERT(!suiteName.isEmpty(), return );

    const SuiteConf conf = SuiteConf::readSuiteConf(
                Utils::FilePath::fromString(m_suites.value(suiteName)));
    const Utils::FilePath objectsMapPath = conf.objectMapPath();
    if (objectsMapPath.exists()) {
        if (!Core::EditorManager::openEditor(objectsMapPath, Constants::OBJECTSMAP_EDITOR_ID)) {
            QMessageBox::critical(Core::ICore::dialogParent(),
                                  Tr::tr("Error"),
                                  Tr::tr("Failed to open objects.map file at \"%1\".")
                                      .arg(objectsMapPath.toUserOutput()));
        }
    }
}

void SquishFileHandler::onSessionLoaded()
{
    // remove currently opened "silently" (without storing into session)
    closeAllInternal();

    const QVariant variant = ProjectExplorer::SessionManager::value(SK_OpenSuites);
    const Utils::FilePaths suitePaths = Utils::transform(variant.toStringList(),
                                                         &Utils::FilePath::fromString);

    // open suites of the old session
    for (const Utils::FilePath &fp : suitePaths) {
        if (fp.exists())
            openTestSuite(fp);
    }
}

} // namespace Internal
} // namespace Squish
