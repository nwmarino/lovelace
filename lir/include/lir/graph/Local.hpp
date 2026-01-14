//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_LOCAL_H_
#define LOVELACE_IR_LOCAL_H_

//
//  This header file declares the Local class, which represents function stack
//  frame locals in the IR.
//

#include "lir/graph/Value.hpp"

#include <cstdint>

namespace lir {

class CFG;
class Function;

class Local final : public Value {
    /// The parent function that this local is in.
    Function* m_parent = nullptr;

    /// The name of this local.
    std::string m_name;

    /// The type allocated for this local.
    Type* m_alloc;

    /// The desired stack alignment of this local. 
    uint32_t m_align;

    Local(PointerType* type, Function* parent, const std::string& name, 
          Type* alloc, uint32_t align)
      : Value(type), m_parent(parent), m_name(name), m_alloc(alloc), 
        m_align(align) {}

public:
    [[nodiscard]] static Local* create(CFG& cfg, Type* type, 
                                       const std::string& name, uint32_t align, 
                                       Function* parent = nullptr);

    void set_parent(Function* parent) { m_parent = parent; }
    const Function* get_parent() const { return m_parent; }
    Function* get_parent() { return m_parent; }

    /// Detaches this local from its parent function. 
    ///
    /// Does not free any memory allocated for this local.
    void detach();

    void set_name(const std::string& name) { m_name = name; }
    const std::string& get_name() const { return m_name; }

    const Type* get_allocated_type() const { return m_alloc; }
    Type* get_allocated_type() { return m_alloc; }

    void set_alignment(uint32_t align) { m_align = align; }
    uint32_t get_alignment() const { return m_align; }

    void print(std::ostream& os) const override;
};

} // namespace lir

#endif // LOVELACE_IR_LOCAL_H_
