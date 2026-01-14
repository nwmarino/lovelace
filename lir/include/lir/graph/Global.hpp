//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_GLOBAL_H_
#define LOVELACE_IR_GLOBAL_H_

//
//  This header file declares the Global class, which is used to represent
//  top-level, global data in the control-flow graph IR.
//

#include "lir/graph/Constant.hpp"

#include <cstdint>

namespace lir {

class CFG;

/// A top-level global variable possibly initialized with a constant.
class Global final : public Constant {
public:
    /// Recognized linkage types for global data.
    enum LinkageType : uint8_t {
        Internal, External,
    };

private:
    /// The parent graph of this function.
    CFG* m_parent;

    /// The linkage type of this named data.
    LinkageType m_linkage;

    /// If true, then this data cannot be mutated after initialization and has
    /// some special lowering requirements.
    bool m_read_only;

    /// The name of this global variable.
    std::string m_name;

    Global(Type* type, CFG* parent, LinkageType linkage, bool read_only, 
           const std::string& name, Constant* init)
      : Constant(type, { init }), m_parent(parent), 
        m_linkage(linkage), m_read_only(read_only), m_name(name) {}

public:
    /// Create a new global of the given |type|. The |read_only| parameter can
    /// ensure no mutations occur to the data. If |read_only| is true, then an
    /// |init| must also be provided.
    static Global* create(CFG& cfg, Type* type, LinkageType linkage, 
                          bool read_only, const std::string& name, 
                          Constant* init = nullptr);

    /// Change the parent graph of this global to |parent|. Does not add this
    /// global to the new parent, nor does it remove it from the old one.
    void set_parent(CFG* parent) { m_parent = parent; }

    const CFG* get_parent() const { return m_parent; }
    CFG* get_parent() { return m_parent; }

    bool has_parent() const { return m_parent != nullptr; }

    void set_linkage(LinkageType linkage) { m_linkage = linkage; }
    LinkageType get_linkage() const { return m_linkage; }

    void set_read_only(bool value = true) { m_read_only = value; }
    bool is_read_only() const { return m_read_only; }

    void set_name(const std::string& name) { m_name = name; }
    const std::string& get_name() const { return m_name; }

    /// Returns the constant initializer of this data, if it exists.
    void set_initializer(Constant* init) { 
        if (has_initializer()) {
            delete m_operands[0];
            m_operands.clear();
        }

        add_operand(init); 
    }

    Constant* get_initializer() const { 
        return has_initializer() 
            ? static_cast<Constant*>(m_operands[0]->get_value()) 
            : nullptr;
    }

    /// Test if this global has a constant initializer.
    bool has_initializer() const { return has_operands(); }

    void print(std::ostream& os) const override;
};

} // namespace lir

#endif // LOVELACE_IR_GLOBAL_H_
