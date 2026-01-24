//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_FUNCTION_H_
#define LOVELACE_IR_FUNCTION_H_

#include "lir/graph/BasicBlock.hpp"
#include "lir/graph/Local.hpp"
#include "lir/graph/Type.hpp"
#include "lir/graph/Value.hpp"

#include <cassert>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace lir {

class CFG;
class Function;

/// Represents an argument to a function.
class FunctionArgument final : public Value {
public:
    /// Possible traits for function arguments.
    enum class Trait : uint32_t {
        None = 0,
        ARet,       //< For aggregate return values passed by pointer.
        Valued,     //< For aggregate arguments passed by pointer.
    };

private:
    Function* m_parent;
    std::string m_name;
    Trait m_trait;

    FunctionArgument(Type* type, Function* parent, const std::string& name, 
                     Trait trait)
      : Value(type), m_parent(parent), m_name(name), m_trait(trait) {}

public:
    [[nodiscard]] static FunctionArgument* create(
        Type* type, const std::string& name, Function* parent = nullptr,
        Trait trait = Trait::None);

    void set_parent(Function* function) { m_parent = function; }
    const Function* get_parent() const { return m_parent; }
    Function* get_parent() { return m_parent; }
    
    bool has_parent() const { return m_parent != nullptr; }

    void set_name(const std::string& name) { m_name = name; }
    const std::string& get_name() const { return m_name; }
    std::string& get_name() { return m_name; }

    /// Test if this argument is named.
    bool has_name() const { return !m_name.empty(); }

    void set_trait(Trait trait) { m_trait = trait; }
    Trait get_trait() const { return m_trait; }
    
    /// Test if this argument has a trait.
    bool has_trait() const { return m_trait != Trait::None; }

    /// Returns the index of this argument in its parent function. Fails if 
    /// this argument does not belong to a function.
    uint32_t get_index() const;

    void print(std::ostream& os, PrintPolicy policy) const override;
};

/// A function routine consisting of basic blocks.
class Function final : public Value {
public:
    /// Recognized linkage types for global functions.
    enum LinkageType : uint8_t {
        Internal = 0, External,
    };

    using Args = std::vector<FunctionArgument*>;
    using Locals = std::map<std::string, Local*>;

private:
    CFG* m_parent;
    LinkageType m_linkage;
    std::string m_name;
    Args m_args;
    Locals m_locals = {};
    BasicBlock* m_head = nullptr;
    BasicBlock* m_tail = nullptr;

    Function(FunctionType* type, CFG* parent, LinkageType linkage, 
             const std::string& name, const Args& args)
      : Value(type), m_parent(parent), m_linkage(linkage), m_name(name), 
        m_args(args) {}

public:
    static Function* create(CFG& cfg, LinkageType linkage, FunctionType* type, 
                            const std::string& name, const Args& args);

    ~Function() override;

    Function(const Function&) = delete;
    void operator=(const Function&) = delete;

    Function(Function&&) noexcept = delete;
    void operator=(Function&&) noexcept = delete;

    void set_linkage(LinkageType linkage) { m_linkage = linkage; }
    LinkageType get_linkage() const { return m_linkage; }
    
    const FunctionType* get_type() const { 
        return static_cast<const FunctionType*>(m_type); 
    }

    const Type* get_return_type() const {
        return get_type()->get_return_type();
    }

    void set_name(const std::string& name) { m_name = name; }
    const std::string& get_name() const { return m_name; }

    void set_parent(CFG* cfg) { m_parent = cfg; }
    void clear_parent() { m_parent = nullptr; }
    const CFG* get_parent() const { return m_parent; }
    CFG* get_parent() { return m_parent; }

    /// Detach this function from its parent graph.
    ///
    /// Does not free any memory allocated for this function.
    void detach();

    const Args& get_args() const { return m_args; }
    Args& get_args() { return m_args; }

    const FunctionArgument* get_arg(uint32_t i) const {
        assert(i < num_args() && "index out of bounds!");
        return m_args[i];
    }

    FunctionArgument* get_arg(uint32_t i) {
        assert(i < num_args() && "index out of bounds!");
        return m_args[i];
    }

    uint32_t num_args() const { return m_args.size(); }
    bool has_args() const { return !m_args.empty(); }

    /// Set the argument at position |i| to |arg|.
    void set_arg(uint32_t i, FunctionArgument* arg);

    /// Append the given |arg| to the back of this functions' argument list.
    void append_arg(FunctionArgument* arg);

    const Locals& get_locals() const { return m_locals; }
    Locals& get_locals() { return m_locals; }

    /// Returns the local in this function with the given |name| if one exists.
    const Local* get_local(const std::string& name) const;
    Local* get_local(const std::string& name) {
        return const_cast<Local*>(
            static_cast<const Function*>(this)->get_local(name));
    }

    /// Add the given |local| to this function. Fails if it would conflict 
    /// name-wise with an existing local in the function.
    void add_local(Local* local);

    /// Remove |local| from this function if it already belongs.
    void remove_local(Local* local);

    /// Returns the first basic block of this function, if one exists.
    void set_head(BasicBlock* block) { m_head = block; }
    const BasicBlock* get_head() const { return m_head; }
    BasicBlock* get_head() { return m_head; }

    /// Returns the last basic block of this function, if one exists.
    void set_tail(BasicBlock* block) { m_tail = block; }
    const BasicBlock* get_tail() const { return m_tail; }
    BasicBlock* get_tail() { return m_tail; }

    /// Prepend the given |block| to the front of this function.
    void prepend(BasicBlock* block);

    /// Append the given |block| to the back of this function.
    void append(BasicBlock* block);

    /// Insert the given |block| into this function at position |i|.
    void insert(BasicBlock* block, uint32_t i);

    /// Insert |block| into this function immediately after |insert_after|. 
    /// Fails if |insert_after| does not already belong to this function.
    void insert(BasicBlock* block, BasicBlock* insert_after);

    /// Remove the basic block |block| if it belongs to this function.
    void remove(BasicBlock* block);

    /// Test if this function has no basic blocks.
    bool empty() const { return m_head == nullptr; }

    /// Returns the size of this function by the number of basic blocks in it.
    uint32_t size() const;

    void print(std::ostream& os, PrintPolicy policy) const override;
};

} // namespace lir

#endif // LOVELACE_IR_FUNCTION_H_
