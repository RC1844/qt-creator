// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <cplusplus/CPlusPlusForwardDeclarations.h>

#include <QStringList>
#include <QVector>

QT_BEGIN_NAMESPACE
class QByteArray;
QT_END_NAMESPACE

namespace CPlusPlus {

class ByteArrayRef;
class Macro;

class CPLUSPLUS_EXPORT MacroArgumentReference
{
  int _bytesOffset;
  int _bytesLength;
  int _utf16charsOffset;
  int _utf16charsLength;

public:
  explicit MacroArgumentReference(int bytesOffset = 0, int bytesLength = 0,
                                  int utf16charsOffset = 0, int utf16charsLength = 0)
    : _bytesOffset(bytesOffset)
    , _bytesLength(bytesLength)
    , _utf16charsOffset(utf16charsOffset)
    , _utf16charsLength(utf16charsLength)
  { }

  int bytesOffset() const
  { return _bytesOffset; }

  int bytesLength() const
  { return _bytesLength; }

  int utf16charsOffset() const
  { return _utf16charsOffset; }

  int utf16charsLength() const
  { return _utf16charsLength; }
};

class CPLUSPLUS_EXPORT Client
{
  Client(const Client &other);
  void operator=(const Client &other);

public:
  enum IncludeType {
    IncludeLocal,
    IncludeGlobal,
    IncludeNext
  };

public:
  Client();
  virtual ~Client() = 0;

  virtual void macroAdded(const Macro &macro) = 0;

  virtual void passedMacroDefinitionCheck(int bytesOffset, int utf16charsOffset,
                                          int line, const Macro &macro) = 0;
  virtual void failedMacroDefinitionCheck(int bytesOffset, int utf16charsOffset,
                                          const ByteArrayRef &name) = 0;

  virtual void notifyMacroReference(int bytesOffset, int utf16charsOffset,
                                    int line, const Macro &macro) = 0;

  virtual void startExpandingMacro(int bytesOffset, int utf16charsOffset,
                                   int line, const Macro &macro,
                                   const QVector<MacroArgumentReference> &actuals
                                            = QVector<MacroArgumentReference>()) = 0;
  virtual void stopExpandingMacro(int bytesOffset, const Macro &macro) = 0; // TODO: ?!

  /// Mark the given macro name as the include guard for the current file.
  virtual void markAsIncludeGuard(const QByteArray &macroName) = 0;

  /// Start skipping from the given utf16charsOffset.
  virtual void startSkippingBlocks(int utf16charsOffset) = 0;
  virtual void stopSkippingBlocks(int utf16charsOffset) = 0;

  virtual void sourceNeeded(int line, const QString &fileName, IncludeType mode,
                            const QStringList &initialIncludes = QStringList()) = 0;

  static inline bool isInjectedFile(const QString &fileName)
  {
      return fileName.startsWith(QLatin1Char('<')) && fileName.endsWith(QLatin1Char('>'));
  }
};

} // namespace CPlusPlus
