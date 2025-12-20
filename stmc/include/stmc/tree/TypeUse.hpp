//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef STATIM_TYPE_USE_H_
#define STATIM_TYPE_USE_H_

#include <cstdint>
#include <string>

namespace stm {

using std::string;

class Type;

/// Represents the use of a type and possible quantifiers over it.
class TypeUse final {
public:
    /// Possible kinds of quantifiers on a type.
    enum Qualifier : uint32_t {
        Mut = 1u << 0,
    };

private:
    /// The underlying type.
    const Type* m_type;

    /// The list of qualifiers for this type.
    uint32_t m_quals;

public:
    TypeUse(const Type* type = nullptr, uint32_t quals = 0)
        : m_type(type), m_quals(quals) {}

    bool operator==(const TypeUse& other) const {
        return m_type == other.m_type && m_quals == other.m_quals;
    }

    operator const Type*() const { return m_type; }

    const Type& operator*() const { return *m_type; }
    const Type* operator->() const { return m_type; }

    /// Compare this type with \p other.
    bool compare(const TypeUse& other) const;

    /// Test if this type can be casted to \p other. The \p implicitly flag
    /// determines if casting should follow implicit or explicit casting rules.
    bool can_cast(const TypeUse& other, bool implicitly = false) const;

    void set_type(const Type* type) { m_type = type; }
    const Type* get_type() const { return m_type; }

    /// Test if this type use has any qualifiers.
    bool is_qualified() const { return m_quals != 0; }

    const uint32_t& get_qualifiers() const { return m_quals; }
    uint32_t& get_qualifiers() { return m_quals; }

    void set_qualifiers(uint32_t quals) { m_quals = quals; }

    /// Clear any qualifiers on this type use.
    void clear_qualifiers() { m_quals = 0; }

    /// Test if this type use is qualified with 'mut'.
    bool is_mut() const { return (m_quals & Mut) != 0; }

    /// Qualify this type use with the 'mut' qualifier.
    void as_mut() { m_quals |= Mut; }

    /// Returns the string equivelant of this type.
    string to_string() const;
};

} // namespace stm

#endif // STATIM_TYPE_USE_H_
