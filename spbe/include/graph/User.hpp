#ifndef SPBE_USER_H_
#define SPBE_USER_H_

#include "Use.hpp"
#include "Value.hpp"

#include <cstdint>
#include <vector>

namespace spbe {

/// A special kind of value that uses other values.
class User : public Value {
protected:
    /// The operands of this user, or "use" edges, that model a use-def chain.
    std::vector<Use*> m_operands = {};

    User() = default;

    User(const std::vector<Value*>& ops, const Type* type) : Value(type) {
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

    /// Returns the operand list of this user.
    const std::vector<Use*>& get_operand_list() const { return m_operands; }
    std::vector<Use*>& get_operand_list() { return m_operands; }

    /// Returns the operand at position |i| of this user.
    const Use* get_operand(uint32_t i) const {
        assert(i <= num_operands() && "index out of bounds!");
        return m_operands[i];
    }
    
    Use* get_operand(uint32_t i) {
        assert(i <= num_operands() && "index out of bounds!");
        return m_operands[i];
    }

    /// Returns the number of operands this user has.
    uint32_t num_operands() const { return m_operands.size(); }

    /// Add a new operand |op| to this user.
    void add_operand(Value* value) {
        m_operands.push_back(new Use(value, this));
    }
};

} // namespace spbe

#endif // SPBE_USER_H_
