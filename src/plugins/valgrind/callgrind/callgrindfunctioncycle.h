// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "callgrindfunction.h"

namespace Valgrind {
namespace Callgrind {

/**
 * self cost of a function cycle: sum of self costs of functions in the cycle
 * callers of a function cycle: set of callers to functions in the cycle
 *                              excluding calls inside the cycle
 * callees of a function cycle: set of callees from functions in the cycle
 *                              excluding calees inside the cycle
 * inclusive cost of a function cycle: sum of inclusive cost of callees of the cycle (see above)
 */
class FunctionCycle : public Function
{
public:
    explicit FunctionCycle(const ParseData *data);
    ~FunctionCycle() override;

    /// sets the list of functions that make up this cycle
    /// NOTE: ownership is *not* transferred to the cycle
    void setFunctions(const QVector<const Function *> &functions);
    /// @return the functions that make up this cycle
    QVector<const Function *> functions() const;

private:
    class Private;
};

} // namespace Callgrind
} // namespace Valgrind
