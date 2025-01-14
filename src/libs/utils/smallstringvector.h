// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "utils_global.h"

#include "smallstring.h"

#include <vector>

#include <QStringList>

namespace Utils {

template<typename String>
class BasicSmallStringVector : public std::vector<String>
{
    using Base = std::vector<String>;

public:
    BasicSmallStringVector() = default;

    using Base::Base;

    explicit BasicSmallStringVector(const Base &stringVector)
        : Base(stringVector.begin(), stringVector.end())
    {
    }

    BasicSmallStringVector(std::initializer_list<String> list)
    {
        Base::reserve(list.size());

        for (auto &&entry : list)
            Base::push_back(std::move(entry));
    }

    explicit BasicSmallStringVector(const QStringList &stringList)
    {
        std::vector<SmallString>::reserve(std::size_t(stringList.count()));

        for (const QString &string : stringList)
            Base::push_back(SmallString::fromQString(string));
    }

    explicit BasicSmallStringVector(const std::vector<std::string> &stringVector)
    {
        Base::reserve(std::size_t(stringVector.size()));

        for (const std::string &string : stringVector)
           Base::emplace_back(string);
    }

    BasicSmallStringVector(const BasicSmallStringVector &) = default;
    BasicSmallStringVector &operator=(const BasicSmallStringVector &) = default;

    BasicSmallStringVector(BasicSmallStringVector &&) noexcept = default;
    BasicSmallStringVector &operator=(BasicSmallStringVector &&)
        noexcept(std::is_nothrow_move_assignable<Base>::value) = default;

    SmallString join(SmallStringView separator) const
    {
        SmallString joinedString;

        joinedString.reserve(totalByteSize() + separator.size() * std::size_t(Base::size()));

        for (auto stringIterator = Base::begin(); stringIterator != Base::end(); ++stringIterator) {
            joinedString.append(*stringIterator);
            if (std::next(stringIterator) != Base::end())
                joinedString.append(separator);
        }

        return joinedString;
    }

    bool contains(SmallStringView string) const noexcept
    {
        return std::find(Base::begin(), Base::end(), string) != Base::end();
    }

    bool removeFast(SmallStringView valueToBeRemoved)
    {
        auto position = std::remove(Base::begin(), Base::end(), valueToBeRemoved);

        const bool hasEntry = position != Base::end();

        erase(position, Base::end());

        return hasEntry;
    }

    void append(String &&string)
    {
        push_back(std::move(string));
    }

    BasicSmallStringVector clone() const { return *this; }

    operator std::vector<std::string>() const
    {
        return std::vector<std::string>(Base::begin(), Base::end());
    }

    operator QStringList() const
    {
        QStringList qStringList;
        qStringList.reserve(int(Base::size()));

        for (const auto &entry : *this)
            qStringList.push_back(QString(entry));

        return qStringList;
    }

private:
    std::size_t totalByteSize() const
    {
        std::size_t totalSize = 0;

        for (auto &&string : *this)
            totalSize += string.size();

        return totalSize;
    }
};



using SmallStringVector = BasicSmallStringVector<BasicSmallString<31>>;
using PathStringVector = BasicSmallStringVector<BasicSmallString<190>>;
using StringViewVector = BasicSmallStringVector<SmallStringView>;
} // namespace Utils;
