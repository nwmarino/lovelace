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

#include <cstdint>
#include <string>

namespace scc {

class Type;

/// Represents the use of a possibly qualified type.
class QualType final {
public:
    /// Possible qualifiers on a type.
    enum class Qualifier : uint32_t {
        Const = 1u << 0, 
        Volatile = 1u << 1,
    };

private:
    /// The underlying type that this refers to, without any qualifiers.
    const Type* m_type = nullptr;

    /// The bitfield of qualifiers on this type, if any.
    uint32_t m_quals = 0;

public:
    QualType() = default;
    QualType(const Type* ty, uint32_t quals = 0) : m_type(ty), m_quals(quals) {}

    QualType(QualType& other) {
        m_type = other.m_type;
        m_quals = other.m_quals;
    }

    QualType(const QualType& other) {
        m_type = other.m_type;
        m_quals = other.m_quals;
    }

    bool operator == (const QualType& other) const;
    bool operator != (const QualType& other) const;

    const Type& operator* () const { return *m_type; }
    const Type* operator-> () const { return m_type; }

    /// Returns the underlying type, without any qualifiers.
    const Type* get_type() const { return m_type; }

    /// Set the underlying type pointer to \p ty.
    void set_type(const Type* ty) { m_type = ty; }

    /// Returns the qualifiers bitfield on this type.
    const uint32_t& qualifiers() const { return m_quals; }
    uint32_t& qualifiers() { return m_quals; }

    /// Returns true if this type has any qualifiers.
    bool is_qualified() const { return m_quals != 0; }

    /// Clear any qualifiers from this type.
    void clear_qualifiers() { m_quals = 0; }

    /// Returns true if this type is qualified with 'const'.
    bool is_const() const {
        return (m_quals & static_cast<uint32_t>(Qualifier::Const)) != 0;
    }

    /// Qualifies this type with 'const', if it isn't already.
    void with_const() {
        m_quals |= static_cast<uint32_t>(Qualifier::Const);
    }

    /// Returns true if this type is qualified with 'volatile'.
    bool is_volatile() const {
        return (m_quals & static_cast<uint32_t>(Qualifier::Volatile)) != 0;
    }

    /// Qualifies this type with 'volatile', if it isn't already.
    void with_volatile() {
        m_quals |= static_cast<uint32_t>(Qualifier::Volatile);
    }

    /// Returns a stringified version of this type.
    std::string to_string() const;
};

} // namespace scc

#endif // SCC_QUALTYPE_H_
