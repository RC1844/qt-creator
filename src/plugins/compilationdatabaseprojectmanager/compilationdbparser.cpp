// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "compilationdbparser.h"

#include "compilationdatabaseconstants.h"

#include <coreplugin/progressmanager/progressmanager.h>
#include <projectexplorer/task.h>
#include <projectexplorer/treescanner.h>
#include <utils/mimeutils.h>
#include <utils/runextensions.h>

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace ProjectExplorer;
using namespace Utils;

namespace CompilationDatabaseProjectManager {
namespace Internal {

CompilationDbParser::CompilationDbParser(const QString &projectName,
                                         const FilePath &projectPath,
                                         const FilePath &rootPath,
                                         MimeBinaryCache &mimeBinaryCache,
                                         BuildSystem::ParseGuard &&guard,
                                         QObject *parent)
    : QObject(parent)
    , m_projectName(projectName)
    , m_projectFilePath(projectPath)
    , m_rootPath(rootPath)
    , m_mimeBinaryCache(mimeBinaryCache)
    , m_guard(std::move(guard))
{
    connect(&m_parserWatcher, &QFutureWatcher<void>::finished, this, [this] {
        m_dbContents = m_parserWatcher.result();
        parserJobFinished();
    });
}

void CompilationDbParser::start()
{
    // Check hash first.
    QFile file(m_projectFilePath.toString());
    if (!file.open(QIODevice::ReadOnly)) {
        finish(ParseResult::Failure);
        return;
    }
    m_projectFileContents = file.readAll();
    const QByteArray newHash = QCryptographicHash::hash(m_projectFileContents,
                                                        QCryptographicHash::Sha1);
    if (m_projectFileHash == newHash) {
        finish(ParseResult::Cached);
        return;
    }
    m_projectFileHash = newHash;
    m_runningParserJobs = 0;

    // Thread 1: Scan disk.
    if (!m_rootPath.isEmpty()) {
        m_treeScanner = new TreeScanner(this);
        m_treeScanner->setFilter([this](const MimeType &mimeType, const FilePath &fn) {
            // Mime checks requires more resources, so keep it last in check list
            bool isIgnored = fn.toString().startsWith(m_projectFilePath.toString() + ".user")
                    || TreeScanner::isWellKnownBinary(mimeType, fn);

            // Cache mime check result for speed up
            if (!isIgnored) {
                auto it = m_mimeBinaryCache.find(mimeType.name());
                if (it != m_mimeBinaryCache.end()) {
                    isIgnored = *it;
                } else {
                    isIgnored = TreeScanner::isMimeBinary(mimeType, fn);
                    m_mimeBinaryCache[mimeType.name()] = isIgnored;
                }
            }

            return isIgnored;
        });
        m_treeScanner->setTypeFactory([](const Utils::MimeType &mimeType, const Utils::FilePath &fn) {
            return TreeScanner::genericFileType(mimeType, fn);
        });
        m_treeScanner->asyncScanForFiles(m_rootPath);
        Core::ProgressManager::addTask(m_treeScanner->future(),
                                       tr("Scan \"%1\" project tree").arg(m_projectName),
                                       "CompilationDatabase.Scan.Tree");
        ++m_runningParserJobs;
        connect(m_treeScanner, &TreeScanner::finished,
                this, &CompilationDbParser::parserJobFinished);
    }

    // Thread 2: Parse the project file.
    const QFuture<DbContents> future = runAsync(&CompilationDbParser::parseProject, this);
    Core::ProgressManager::addTask(future,
                                   tr("Parse \"%1\" project").arg(m_projectName),
                                   "CompilationDatabase.Parse");
    ++m_runningParserJobs;
    m_parserWatcher.setFuture(future);
}

void CompilationDbParser::stop()
{
    disconnect();
    m_parserWatcher.disconnect();
    m_parserWatcher.cancel();
    if (m_treeScanner) {
        m_treeScanner->disconnect();
        m_treeScanner->future().cancel();
    }
    m_guard = {};
    deleteLater();
}

QList<FileNode *> CompilationDbParser::scannedFiles() const
{
    const bool canceled = m_treeScanner->future().isCanceled();
    const TreeScanner::Result result = m_treeScanner->release();
    return !canceled ? result.allFiles : QList<FileNode *>();
}

void CompilationDbParser::parserJobFinished()
{
    if (--m_runningParserJobs == 0)
        finish(ParseResult::Success);
}

void CompilationDbParser::finish(ParseResult result)
{
    if (result != ParseResult::Failure)
        m_guard.markAsSuccess();

    emit finished(result);
    deleteLater();
}

static QStringList jsonObjectFlags(const QJsonObject &object, QSet<QString> &flagsCache)
{
    QStringList flags;
    const QJsonArray arguments = object["arguments"].toArray();
    if (arguments.isEmpty()) {
        flags = splitCommandLine(object["command"].toString(), flagsCache);
    } else {
        flags.reserve(arguments.size());
        for (const QJsonValue &arg : arguments) {
            auto flagIt = flagsCache.insert(arg.toString());
            flags.append(*flagIt);
        }
    }

    return flags;
}

static FilePath jsonObjectFilePath(const QJsonObject &object)
{
    const QString workingDir = QDir::cleanPath(object["directory"].toString());
    FilePath fileName = FilePath::fromString(QDir::cleanPath(object["file"].toString()));
    if (fileName.toFileInfo().isRelative())
        fileName = FilePath::fromString(QDir::cleanPath(workingDir + "/" + fileName.toString()));
    return fileName;
}

std::vector<DbEntry> CompilationDbParser::readJsonObjects() const
{
    std::vector<DbEntry> result;

    int objectStart = m_projectFileContents.indexOf('{');
    int objectEnd = m_projectFileContents.indexOf('}', objectStart + 1);

    QSet<QString> flagsCache;
    while (objectStart >= 0 && objectEnd >= 0) {
        const QJsonDocument document = QJsonDocument::fromJson(
                    m_projectFileContents.mid(objectStart, objectEnd - objectStart + 1));
        if (document.isNull()) {
            // The end was found incorrectly, search for the next one.
            objectEnd = m_projectFileContents.indexOf('}', objectEnd + 1);
            continue;
        }

        const QJsonObject object = document.object();
        const Utils::FilePath filePath = jsonObjectFilePath(object);
        const QStringList flags = filterFromFileName(jsonObjectFlags(object, flagsCache),
                                                     filePath.fileName());
        result.push_back({flags, filePath, object["directory"].toString()});

        objectStart = m_projectFileContents.indexOf('{', objectEnd + 1);
        objectEnd = m_projectFileContents.indexOf('}', objectStart + 1);
    }

    return result;
}

QStringList readExtraFiles(const QString &filePath)
{
    QStringList result;

    QFile file(filePath);
    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);

        while (!stream.atEnd()) {
            QString line = stream.readLine();
            line = line.trimmed();

            if (line.isEmpty() || line.startsWith('#'))
                continue;

            result.push_back(line);
        }
    }

    return result;
}

DbContents CompilationDbParser::parseProject()
{
    DbContents dbContents;
    dbContents.entries = readJsonObjects();
    dbContents.extraFileName = m_projectFilePath.toString() +
            Constants::COMPILATIONDATABASEPROJECT_FILES_SUFFIX;
    dbContents.extras = readExtraFiles(dbContents.extraFileName);
    std::sort(dbContents.entries.begin(), dbContents.entries.end(),
              [](const DbEntry &lhs, const DbEntry &rhs) {
        return std::lexicographical_compare(lhs.flags.begin(), lhs.flags.end(),
                                            rhs.flags.begin(), rhs.flags.end());
    });
    return dbContents;
}

} // namespace Internal
} // namespace CompilationDatabaseProjectManager
