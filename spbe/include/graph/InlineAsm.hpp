#ifndef SPBE_INLINE_ASM_H_
#define SPBE_INLINE_ASM_H_

#include "graph/Value.hpp"

namespace spbe {

/// Represents an inline assembly block.
class InlineAsm final : public Value {
    /// The assembly string for this inline asm.
    std::string m_iasm;

    /// The list of constraints for this inline asm.
    std::vector<std::string> m_constraints;

    /// If true, then this assembly block has been marked with some sort of 
    /// side effects that means it cannot be optimized away or reordered freely.
    bool m_side_effects;

public:
    InlineAsm(const FunctionType* type, const std::string& iasm, 
              const std::vector<std::string>& constraints = {}, 
              bool side_effects = false)
        : Value(type), m_iasm(iasm), m_constraints(constraints), 
          m_side_effects(side_effects) {}

    InlineAsm(const InlineAsm&) = delete;
    InlineAsm& operator = (const InlineAsm&) = delete;

    /// Return the assembly string for this inline asm.
    const std::string& string() const { return m_iasm; }

    /// Returns the list of constraints for this inline asm.
    const std::vector<std::string>& constraints() const { 
        return m_constraints; 
    }

    /// Returns true if this inline asm is said to have side effects.
    bool has_side_effects() const { return m_side_effects; }

    void print(std::ostream& os) const override;
};

} // namespace spbe

#endif // SPBE_INLINE_ASM_H_
