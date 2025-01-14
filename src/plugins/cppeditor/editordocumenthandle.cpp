// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "editordocumenthandle.h"

namespace CppEditor {

/*!
    \class CppEditor::EditorDocumentHandle

    \brief The EditorDocumentHandle class provides an interface to an opened
           C++ editor document.
*/

CppEditorDocumentHandle::~CppEditorDocumentHandle() = default;

SendDocumentTracker &CppEditorDocumentHandle::sendTracker()
{
    return m_sendTracker;
}

CppEditorDocumentHandle::RefreshReason CppEditorDocumentHandle::refreshReason() const
{
    return m_refreshReason;
}

void CppEditorDocumentHandle::setRefreshReason(const RefreshReason &refreshReason)
{
    m_refreshReason = refreshReason;
}

} // namespace CppEditor
