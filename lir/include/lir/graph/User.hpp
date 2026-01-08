//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_USER_H_
#define LOVELACE_IR_USER_H_

//
//  This header file declares the User class, which is a special kind of Value
//  that may or may not make use of other values.
//

#include "lir/graph/Use.hpp"
#include "lir/graph/Value.hpp"

#include <cstdint>
#include <vector>

namespace lir {

/// A special kind of value that uses other values.
class User : public Value {
protected:
    /// The operands of this user, or "use" edges, that model a use-def chain.
    Uses m_operands = {};

    User(Type* type, const std::vector<Value*>& ops = {}) : Value(type) {
        for (Value* const& v : ops)
            if (v) m_operands.emplace_back(new Use(v, this));
    }

public:
    ~User() {
        for (Use*& use : m_operands) {
            delete use;
            use = nullptr;
        }

        m_operands.clear();
    }

    const Uses& get_operand_list() const { return m_operands; }
    Uses& get_operand_list() { return m_operands; }

    const Use* get_operand(uint32_t i) const {
        assert(i < num_operands() && "index out of bounds!");
        return m_operands[i];
    }
    
    Use* get_operand(uint32_t i) {
        assert(i < num_operands() && "index out of bounds!");
        return m_operands[i];
    }

    uint32_t num_operands() const { return m_operands.size(); }
    bool has_operands() const { return !m_operands.empty(); }

    /// Add the given |value| as a new operand to this user.
    void add_operand(Value* value) {
        m_operands.push_back(new Use(value, this));
    }
};

} // namespace lir

#endif // LOVELACE_IR_USER_H_
