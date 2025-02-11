// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "cppeditor_global.h"

#include <cplusplus/ASTPath.h>
#include <cplusplus/CppDocument.h>
#include <cplusplus/TranslationUnit.h>

#include <QObject>
#include <QTextCursor>

namespace CppEditor {

class ASTNodePositions {
public:
    ASTNodePositions() = default;
    explicit ASTNodePositions(CPlusPlus::AST *_ast) : ast(_ast) {}
    operator bool() const { return ast; }

    CPlusPlus::AST *ast = nullptr;
    unsigned firstTokenIndex = 0;
    unsigned lastTokenIndex = 0;
    unsigned secondToLastTokenIndex = 0;
    int astPosStart = -1;
    int astPosEnd = -1;
};

class CPPEDITOR_EXPORT CppSelectionChanger : public QObject
{
    Q_OBJECT
public:
    explicit CppSelectionChanger(QObject *parent = nullptr);

    enum Direction {
        ExpandSelection,
        ShrinkSelection
    };

    enum NodeIndexAndStepState {
        NodeIndexAndStepNotSet,
        NodeIndexAndStepWholeDocument,
    };

    bool changeSelection(Direction direction,
                         QTextCursor &cursorToModify,
                         const CPlusPlus::Document::Ptr doc);
    void startChangeSelection();
    void stopChangeSelection();

public slots:
    void onCursorPositionChanged(const QTextCursor &newCursor);

protected slots:
    void fineTuneForStatementPositions(unsigned firstParensTokenIndex,
                                       unsigned lastParensTokenIndex,
                                       ASTNodePositions &positions) const;

private:
    bool performSelectionChange(QTextCursor &cursorToModify);
    ASTNodePositions getASTPositions(CPlusPlus::AST *ast, const QTextCursor &cursor) const;
    void updateCursorSelection(QTextCursor &cursorToModify, ASTNodePositions positions);

    int possibleASTStepCount(CPlusPlus::AST *ast) const;
    int currentASTStep() const;
    ASTNodePositions findNextASTStepPositions(const QTextCursor &cursor);

    void fineTuneASTNodePositions(ASTNodePositions &positions) const;
    ASTNodePositions getFineTunedASTPositions(CPlusPlus::AST *ast, const QTextCursor &cursor) const;
    int getFirstCurrentStepForASTNode(CPlusPlus::AST *ast) const;
    bool isLastPossibleStepForASTNode(CPlusPlus::AST *ast) const;
    ASTNodePositions findRelevantASTPositionsFromCursor(const QList<CPlusPlus::AST *> &astPath,
                                              const QTextCursor &cursor,
                                              int startingFromNodeIndex = -1);
    ASTNodePositions findRelevantASTPositionsFromCursorWhenNodeIndexNotSet(
            const QList<CPlusPlus::AST *> &astPath,
            const QTextCursor &cursor);
    ASTNodePositions findRelevantASTPositionsFromCursorWhenWholeDocumentSelected(
            const QList<CPlusPlus::AST *> &astPath,
            const QTextCursor &cursor);
    ASTNodePositions findRelevantASTPositionsFromCursorFromPreviousNodeIndex(
            const QList<CPlusPlus::AST *> &astPath,
            const QTextCursor &cursor);
    bool shouldSkipASTNodeBasedOnPosition(const ASTNodePositions &positions,
                                          const QTextCursor &cursor) const;
    void setNodeIndexAndStep(NodeIndexAndStepState state);
    int getTokenStartCursorPosition(unsigned tokenIndex, const QTextCursor &cursor) const;
    int getTokenEndCursorPosition(unsigned tokenIndex, const QTextCursor &cursor) const;
    void printTokenDebugInfo(unsigned tokenIndex, const QTextCursor &cursor, QString prefix) const;

    QTextCursor m_initialChangeSelectionCursor;
    QTextCursor m_workingCursor;
    CPlusPlus::Document::Ptr m_doc;
    CPlusPlus::TranslationUnit *m_unit = nullptr;
    Direction m_direction = ExpandSelection;
    int m_changeSelectionNodeIndex = -1;
    int m_nodeCurrentStep = -1;
    bool m_inChangeSelection = false;
};

} // namespace CppEditor
