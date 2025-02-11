// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qmljs/parser/qmljsglobal_p.h"
#include "qmljs/parser/qmljsgrammar_p.h"

#include <QtCore/qstring.h>
#include <QtCore/qstack.h>

QT_QML_BEGIN_NAMESPACE

namespace QmlJS {

class Engine;
class DiagnosticMessage;
class Directives;

class QML_PARSER_EXPORT Lexer: public QmlJSGrammar
{
public:
    enum {
        T_ABSTRACT = T_RESERVED_WORD,
        T_BOOLEAN = T_RESERVED_WORD,
        T_BYTE = T_RESERVED_WORD,
        T_CHAR = T_RESERVED_WORD,
        T_DOUBLE = T_RESERVED_WORD,
        T_FINAL = T_RESERVED_WORD,
        T_FLOAT = T_RESERVED_WORD,
        T_GOTO = T_RESERVED_WORD,
        T_IMPLEMENTS = T_RESERVED_WORD,
        T_INT = T_RESERVED_WORD,
        T_INTERFACE = T_RESERVED_WORD,
        T_LONG = T_RESERVED_WORD,
        T_NATIVE = T_RESERVED_WORD,
        T_PACKAGE = T_RESERVED_WORD,
        T_PRIVATE = T_RESERVED_WORD,
        T_PROTECTED = T_RESERVED_WORD,
        T_SHORT = T_RESERVED_WORD,
        T_SYNCHRONIZED = T_RESERVED_WORD,
        T_THROWS = T_RESERVED_WORD,
        T_TRANSIENT = T_RESERVED_WORD,
        T_VOLATILE = T_RESERVED_WORD
    };

    enum Error {
        NoError,
        IllegalCharacter,
        IllegalNumber,
        UnclosedStringLiteral,
        IllegalEscapeSequence,
        IllegalUnicodeEscapeSequence,
        UnclosedComment,
        IllegalExponentIndicator,
        IllegalIdentifier,
        IllegalHexadecimalEscapeSequence
    };

    enum RegExpBodyPrefix {
        NoPrefix,
        EqualPrefix
    };

    enum RegExpFlag {
        RegExp_Global     = 0x01,
        RegExp_IgnoreCase = 0x02,
        RegExp_Multiline  = 0x04,
        RegExp_Unicode    = 0x08,
        RegExp_Sticky     = 0x10
    };

    enum ParseModeFlags {
        QmlMode = 0x1,
        YieldIsKeyword = 0x2,
        StaticIsKeyword = 0x4
    };

    enum class ImportState {
        SawImport,
        NoQmlImport
    };

public:
    Lexer(Engine *engine);

    int parseModeFlags() const {
        int flags = 0;
        if (qmlMode())
            flags |= QmlMode|StaticIsKeyword;
        if (yieldIsKeyWord())
            flags |= YieldIsKeyword;
        if (_staticIsKeyword)
            flags |= StaticIsKeyword;
        return flags;
    }

    bool qmlMode() const;
    bool yieldIsKeyWord() const { return _generatorLevel != 0; }
    void setStaticIsKeyword(bool b) { _staticIsKeyword = b; }

    QString code() const;
    void setCode(const QString &code, int lineno, bool qmlMode = true);

    int lex();

    bool scanRegExp(RegExpBodyPrefix prefix = NoPrefix);
    bool scanDirectives(Directives *directives, DiagnosticMessage *error);

    int regExpFlags() const { return _patternFlags; }
    QString regExpPattern() const { return _tokenText; }

    int tokenKind() const { return _tokenKind; }
    int tokenOffset() const { return _tokenStartPtr - _code.unicode(); }
    int tokenLength() const { return _tokenLength; }

    int tokenStartLine() const { return _tokenLine; }
    int tokenStartColumn() const { return _tokenColumn; }

    inline QStringView tokenSpell() const { return _tokenSpell; }
    inline QStringView rawString() const { return _rawString; }
    double tokenValue() const { return _tokenValue; }
    QString tokenText() const;

    Error errorCode() const;
    QString errorMessage() const;

    bool prevTerminator() const;
    bool followsClosingBrace() const;
    bool canInsertAutomaticSemicolon(int token) const;

    enum ParenthesesState {
        IgnoreParentheses,
        CountParentheses,
        BalancedParentheses
    };

    void enterGeneratorBody() { ++_generatorLevel; }
    void leaveGeneratorBody() { --_generatorLevel; }

protected:
    static int classify(const QChar *s, int n, int parseModeFlags);

private:
    inline void scanChar();
    inline QChar peekChar();
    int scanToken();
    int scanNumber(QChar ch);
    int scanVersionNumber(QChar ch);
    enum ScanStringMode {
        SingleQuote = '\'',
        DoubleQuote = '"',
        TemplateHead = '`',
        TemplateContinuation = 0
    };
    int scanString(ScanStringMode mode);

    bool isLineTerminator() const;
    unsigned isLineTerminatorSequence() const;
    static bool isIdentLetter(QChar c);
    static bool isDecimalDigit(ushort c);
    static bool isHexDigit(QChar c);
    static bool isOctalDigit(ushort c);

    void syncProhibitAutomaticSemicolon();
    uint decodeUnicodeEscapeCharacter(bool *ok);
    QChar decodeHexEscapeCharacter(bool *ok);

private:
    Engine *_engine;

    QString _code;
    QString _tokenText;
    QString _errorMessage;
    QStringView _tokenSpell;
    QStringView _rawString;

    const QChar *_codePtr;
    const QChar *_endPtr;
    const QChar *_tokenStartPtr;

    QChar _char;
    Error _errorCode;

    int _currentLineNumber;
    int _currentColumnNumber;
    double _tokenValue;

    // parentheses state
    ParenthesesState _parenthesesState;
    int _parenthesesCount;

    // template string stack
    QStack<int> _outerTemplateBraceCount;
    int _bracesCount = -1;

    int _stackToken;

    int _patternFlags;
    int _tokenKind;
    int _tokenLength;
    int _tokenLine;
    int _tokenColumn;
    ImportState _importState = ImportState::NoQmlImport;

    bool _validTokenText;
    bool _prohibitAutomaticSemicolon;
    bool _restrictedKeyword;
    bool _terminator;
    bool _followsClosingBrace;
    bool _delimited;
    bool _qmlMode;
    bool _skipLinefeed = false;
    int _generatorLevel = 0;
    bool _staticIsKeyword = false;
    bool _handlingDirectives = false;
};

} // end of namespace QmlJS

QT_QML_END_NAMESPACE

