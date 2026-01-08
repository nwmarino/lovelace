//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_CFG_H_
#define LOVELACE_IR_CFG_H_

#include "lir/graph/BasicBlock.hpp"
#include "lir/graph/Constant.hpp"
#include "lir/graph/Function.hpp"
#include "lir/graph/Global.hpp"
#include "lir/graph/Type.hpp"
#include "lir/machine/Machine.hpp"

#include <map>
#include <ostream>
#include <string>
#include <unordered_map>

namespace lir {

class CFG final {
    friend class Type;
    friend class ArrayType;
    friend class FloatType;
    friend class FunctionType;
    friend class IntegerType;
    friend class PointerType;
    friend class StructType;
    friend class VoidType;

    friend class Integer;
    friend class Float;
    friend class Null;
    friend class String;
    friend class BlockAddress;

    using Functions = std::map<std::string, Function*>;
    using Globals = std::map<std::string, Global*>;

    using ArrayTypePool = std::unordered_map<
        const Type*, std::unordered_map<uint32_t, ArrayType*>>;
    using FloatTypePool = std::unordered_map<uint32_t, FloatType*>;
    using FunctionTypePool = std::vector<FunctionType*>;
    using IntegerTypePool = std::unordered_map<uint32_t, IntegerType*>;
    using PointerTypePool = std::unordered_map<const Type*, PointerType*>;
    using StructTypePool = std::map<std::string, StructType*>;

    using BytePool = std::unordered_map<int8_t, Integer*>;
    using ShortPool = std::unordered_map<int16_t, Integer*>;
    using IntPool = std::unordered_map<int32_t, Integer*>;
    using LongPool = std::unordered_map<int64_t, Integer*>;
    using FloatPool = std::unordered_map<float, Float*>;
    using DoublePool = std::unordered_map<double, Float*>;
    using NullPool = std::unordered_map<const Type*, Null*>;
    using StringPool = std::unordered_map<std::string, String*>;
    using AddressPool = std::unordered_map<const BasicBlock*, BlockAddress*>;

    const Machine& m_mach;

    std::string m_filename;
    uint32_t m_def_id = 1;
    Globals m_globals = {};
    Functions m_functions = {};

    struct TypePools final {
        VoidType* void_type = nullptr;
        ArrayTypePool arrays = {};
        FloatTypePool floats = {};
        FunctionTypePool functions = {};
        IntegerTypePool ints = {};
        PointerTypePool pointers = {};
        StructTypePool structs = {};
    } m_types;

    struct ConstantPools final {
        Integer* zero;
        Integer* one;
        BytePool bytes = {};
        ShortPool shorts = {};
        IntPool ints = {};
        LongPool longs = {};
        FloatPool floats = {};
        DoublePool doubles = {};
        NullPool nulls = {};
        StringPool strings = {};
        AddressPool addresses = {};
    } m_constants;

public:
    CFG(const Machine& mach, const std::string& filename);

    ~CFG();
    
    CFG(const CFG&) = delete;
    void operator=(const CFG&) = delete;
    
    CFG(CFG&&) noexcept = delete;
    void operator=(CFG&&) noexcept = delete;

    const Machine& get_machine() const { return m_mach; }

    void set_filename(std::string& filename) { m_filename = filename; }
    const std::string& get_filename() const { return m_filename; }
    std::string& get_filename() { return m_filename; }
    
    std::vector<StructType*> get_structs() const;

    std::vector<Global*> get_globals() const;

    /// Returns the global in this graph with the given |name|.
    const Global* get_global(const std::string& name) const;
    Global* get_global(const std::string& name) {
        return const_cast<Global*>(
            static_cast<const CFG*>(this)->get_global(name));
    }

    /// Add the given |global| to this graph. Fails if there is any existing 
    /// top-level symbol with the same name.
    void add_global(Global* global);

    /// Remove the given |global| from this graph, if it belongs.
    void remove_global(Global* global);

    /// Returns a list of all functions in this graph, in order of addition.
    std::vector<Function*> get_functions() const;

    /// Returns the function in this graph with the given |name|.
    const Function* get_function(const std::string& name) const;
    Function* get_function(const std::string& name) {
        return const_cast<Function*>(
            static_cast<const CFG*>(this)->get_function(name));
    }

    /// Add the given |function| to this graph. Fails if there is any existing 
    /// top-level symbol with the same name.
    void add_function(Function* function);

    /// Remove the given |function| from this graph, if it belongs.
    void remove_function(Function* function);

    /// Return a new unique definition id to create an instruction with.
    uint32_t get_def_id() { return m_def_id++; }

    /// Print this graph in a plaintext format to |os|.
    void print(std::ostream& os) const;
};

} // namespace lir

#endif // LOVELACE_IR_CFG_H_
