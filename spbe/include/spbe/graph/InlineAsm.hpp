//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SPBE_INLINE_ASM_H_
#define SPBE_INLINE_ASM_H_

#include "spbe/graph/Value.hpp"

namespace spbe {

/// Represents an inline assembly block.
class InlineAsm final : public Value {
    /// The assembly string for this inline asm.
    std::string m_iasm;

    /// The list of constraints for this inline asm.
    std::vector<std::string> m_constraints;

public:
    InlineAsm(const FunctionType* type, const std::string& iasm, 
              const std::vector<std::string>& constraints = {})
        : Value(type), m_iasm(iasm), m_constraints(constraints) {}

    InlineAsm(const InlineAsm&) = delete;
    InlineAsm& operator = (const InlineAsm&) = delete;

    /// Return the assembly string for this inline asm.
    const std::string& string() const { return m_iasm; }

    /// Returns the list of constraints for this inline asm.
    const std::vector<std::string>& constraints() const { 
        return m_constraints; 
    }

    void print(std::ostream& os) const override;
};

} // namespace spbe

#endif // SPBE_INLINE_ASM_H_
