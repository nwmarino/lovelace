//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SCC_QUALTYPE_H_
#define SCC_QUALTYPE_H_

//
// This header file defines an important structure to represent qualified types
// i.e. types that might have qualifiers applied to them.
//
// Qualifiers in C are keywords like 'const' or 'volatile' that have effects
// on the types they are applied to.
//

#include "ast/Type.hpp"

#include <algorithm>

namespace scc {

/// Represents the use of a possibly qualified type.
class QualType final {
public:
    /// Possible qualifiers on a type.
    enum class Qualifier : uint8_t {
        Const, Volatile,
    };

    using QualifierList = std::vector<Qualifier>;

private:
    /// The underlying type that this refers to, without any qualifiers.
    const Type* m_type = nullptr;

    /// The list of qualifiers on this type, if any.
    QualifierList m_quals = {};

public:
    QualType(const Type* ty) : m_type(ty) {}

    ~QualType() = default;

    const Type& operator* () const { return *m_type; }
    const Type* operator-> () const { return m_type; }

    /// Returns the underlying type, without any qualifiers.
    const Type* get_type() const { return m_type; }

    /// Returns the list of qualifiers on this type.
    const QualifierList& qualifiers() const { return m_quals; }
    QualifierList& qualifiers() { return m_quals; }

    /// Returns true if this type has any qualifiers.
    bool is_qualified() const { return !m_quals.empty(); }

    /// Clear any qualifiers from this type.
    void clear_qualifiers() { m_quals.clear(); }

    /// Returns true if this type is qualified with 'const'.
    bool is_const() const {
        return std::find(
            m_quals.begin(), m_quals.end(), Qualifier::Const) != m_quals.end();
    }

    /// Qualifies this type with 'const', if it isn't already.
    void with_const() {
        if (!is_const())
            m_quals.push_back(Qualifier::Const);
    }

    /// Returns true if this type is qualified with 'volatile'.
    bool is_volatile() const {
        return std::find(
            m_quals.begin(), m_quals.end(), Qualifier::Volatile) != m_quals.end();
    }

    /// Qualifies this type with 'volatile', if it isn't already.
    void with_volatile() {
        if (!is_volatile())
            m_quals.push_back(Qualifier::Volatile);
    }
};

} // namespace scc

#endif // SCC_QUALTYPE_H_
