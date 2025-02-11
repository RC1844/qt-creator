// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "profilehoverhandler.h"
#include "profilecompletionassist.h"
#include "qmakeprojectmanagerconstants.h"

#include <coreplugin/helpmanager.h>
#include <texteditor/texteditor.h>
#include <utils/htmldocextractor.h>
#include <utils/executeondestruction.h>

#include <QTextBlock>
#include <QUrl>

using namespace Core;
using namespace Utils;

namespace QmakeProjectManager {
namespace Internal {

ProFileHoverHandler::ProFileHoverHandler()
    : m_keywords(qmakeKeywords())
{
}

void ProFileHoverHandler::identifyMatch(TextEditor::TextEditorWidget *editorWidget,
                                        int pos,
                                        ReportPriority report)
{
    ExecuteOnDestruction reportPriority([this, report](){ report(priority()); });

    m_docFragment.clear();
    m_manualKind = UnknownManual;
    if (!editorWidget->extraSelectionTooltip(pos).isEmpty()) {
        setToolTip(editorWidget->extraSelectionTooltip(pos));
    } else {
        QTextDocument *document = editorWidget->document();
        QTextBlock block = document->findBlock(pos);
        identifyQMakeKeyword(block.text(), pos - block.position());

        if (m_manualKind != UnknownManual) {
            QUrl url(QString::fromLatin1("qthelp://org.qt-project.qmake/qmake/qmake-%1-reference.html#%2")
                     .arg(manualName()).arg(m_docFragment));
            setLastHelpItemIdentified(
                Core::HelpItem(url, m_docFragment, Core::HelpItem::QMakeVariableOfFunction));
        } else {
            // General qmake manual will be shown outside any function or variable
            setLastHelpItemIdentified("qmake");
        }
    }
}

void ProFileHoverHandler::identifyQMakeKeyword(const QString &text, int pos)
{
    if (text.isEmpty())
        return;

    QString buf;

    for (int i = 0; i < text.length(); ++i) {
        const QChar c = text.at(i);
        bool checkBuffer = false;
        if (c.isLetter() || c == QLatin1Char('_') || c == QLatin1Char('.') || c.isDigit()) {
            buf += c;
            if (i == text.length() - 1)
                checkBuffer = true;
        } else {
            checkBuffer = true;
        }
        if (checkBuffer) {
            if (!buf.isEmpty()) {
                if ((i >= pos) && (i - buf.size() <= pos)) {
                    if (m_keywords.isFunction(buf))
                        identifyDocFragment(FunctionManual, buf);
                    else if (m_keywords.isVariable(buf))
                        identifyDocFragment(VariableManual, buf);
                    break;
                }
                buf.clear();
            } else {
                if (i >= pos)
                    break; // we are after the tooltip pos
            }
            if (c == QLatin1Char('#'))
                break; // comment start
        }
    }
}

QString ProFileHoverHandler::manualName() const
{
    if (m_manualKind == FunctionManual)
        return QLatin1String("function");
    else if (m_manualKind == VariableManual)
        return QLatin1String("variable");
    return QString();
}

void ProFileHoverHandler::identifyDocFragment(ProFileHoverHandler::ManualKind manualKind,
                                        const QString &keyword)
{
    m_manualKind = manualKind;
    m_docFragment = keyword.toLower();
    // Special case: _PRO_FILE_ and _PRO_FILE_PWD_ ids
    // don't have starting and ending '_'.
    if (m_docFragment.startsWith(QLatin1Char('_')))
        m_docFragment = m_docFragment.mid(1);
    if (m_docFragment.endsWith(QLatin1Char('_')))
        m_docFragment = m_docFragment.left(m_docFragment.size() - 1);
    m_docFragment.replace(QLatin1Char('.'), QLatin1Char('-'));
    m_docFragment.replace(QLatin1Char('_'), QLatin1Char('-'));

    if (m_manualKind == FunctionManual) {
        QUrl url(QString::fromLatin1("qthelp://org.qt-project.qmake/qmake/qmake-%1-reference.html").arg(manualName()));
        const QByteArray html = Core::HelpManager::fileData(url);

        HtmlDocExtractor htmlExtractor;
        htmlExtractor.setMode(HtmlDocExtractor::FirstParagraph);

        // Document fragment of qmake function is retrieved from docs.
        // E.g. in case of the keyword "find" the document fragment
        // parsed from docs is "find-variablename-substr".
        m_docFragment = htmlExtractor.getQMakeFunctionId(QString::fromUtf8(html), m_docFragment);
    }
}

} // namespace Internal
} // namespace QmakeProjectManager
