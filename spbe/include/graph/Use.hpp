#ifndef SPBE_USE_H_
#define SPBE_USE_H_

#include "Value.hpp"

namespace spbe {

class User;

/// Represents a use; the edge between a value and a user of it.
class Use final {
    /// The value being used.
    Value* m_value;

    /// The value/user that is using the value in the edge.
    User* m_user;

public:
    /// Create a new use edge between a value and a user.
    Use(Value* value, User* user);

    Use(const Use&) = delete;
    Use& operator = (const Use&) = delete;

    ~Use() {
        m_value->del_use(this);
        m_value = nullptr;
        m_user = nullptr;
    }

    operator Value*() { return m_value; }

    operator User*() { return m_user; }

    /// Returns the value of this use edge.
    const Value* get_value() const { return m_value; }
    Value* get_value() { return m_value; }

    /// Set the value of this use to |value|.
    void set_value(Value* value) {
        assert(m_value && "current use value is null!");
        assert(value && "new use value cannot be null!");

        if (m_value == value) 
            return;

        m_value->del_use(this);
        m_value = value;
        m_value->add_use(this);
    }

    /// Returns the user of this use edge.
    const User* get_user() const { return m_user; }
    User* get_user() { return m_user; }
};

} // namespace spbe

#endif // SPBE_USE_H_
