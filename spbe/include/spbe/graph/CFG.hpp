//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SPBE_CFG_H_
#define SPBE_CFG_H_

#include "spbe/graph/BasicBlock.hpp"
#include "spbe/graph/Constant.hpp"
#include "spbe/graph/Function.hpp"
#include "spbe/graph/Global.hpp"
#include "spbe/graph/Type.hpp"

#include <map>
#include <ostream>
#include <string>
#include <unordered_map>

namespace spbe {

class Target;

/// The top-level SIIR control flow graph.
class CFG final {
    using IntegerTypePool = std::unordered_map<IntegerType::Kind, IntegerType*>;
    using FloatTypePool = std::unordered_map<FloatType::Kind, FloatType*>;
    using ArrayTypePool = std::unordered_map<const Type*, 
        std::unordered_map<uint32_t, ArrayType*>>;
    using PointerTypePool = std::unordered_map<const Type*, PointerType*>;
    using StructTypePool = std::map<std::string, StructType*>;
    using FunctionTypePool = std::vector<FunctionType*>;

    using Int8Pool = std::unordered_map<int8_t, ConstantInt*>;
    using Int16Pool = std::unordered_map<int16_t, ConstantInt*>;
    using Int32Pool = std::unordered_map<int32_t, ConstantInt*>;
    using Int64Pool = std::unordered_map<int64_t, ConstantInt*>;
    using FloatPool = std::unordered_map<float, ConstantFP*>;
    using DoublePool = std::unordered_map<double, ConstantFP*>;
    using NullPool = std::unordered_map<const Type*, ConstantNull*>;
    using BlockAddrPool = std::unordered_map<const BasicBlock*, BlockAddress*>;
    using StringPool = std::unordered_map<std::string, ConstantString*>;

    friend class Type;
    friend class ArrayType;
    friend class FunctionType;
    friend class PointerType;
    friend class StructType;
    friend class ConstantInt;
    friend class ConstantFP;
    friend class ConstantNull;
    friend class BlockAddress;
    friend class ConstantString;
    friend class Instruction;

    /// Top-level graph items.
    std::string m_file;
    Target& m_target;
    uint32_t m_def_id = 1;
    std::map<std::string, Global*> m_globals = {};
    std::map<std::string, Function*> m_functions = {};

    /// Type pooling.
    IntegerTypePool m_types_ints = {};
    FloatTypePool m_types_floats = {};
    ArrayTypePool m_types_arrays = {};
    PointerTypePool m_types_ptrs = {};
    StructTypePool m_types_structs = {};
    FunctionTypePool m_types_fns = {}; 

    /// Constant pooling.
    ConstantInt *m_int1_zero, *m_int1_one;
    Int8Pool m_pool_int8 = {};
    Int16Pool m_pool_int16 = {};
    Int32Pool m_pool_int32 = {};
    Int64Pool m_pool_int64 = {};
    FloatPool m_pool_fp32 = {};
    DoublePool m_pool_fp64 = {};
    NullPool m_pool_null = {};
    BlockAddrPool m_pool_baddr = {};
    StringPool m_pool_str = {};

    /// PHI operand pooling. This is only here because the memory cannot be 
    /// appropriately managed by the individual instructions. TODO: Change.
    std::vector<PhiOperand*> m_pool_incomings = {};

public:
    /// Create a new control flow graph representing |file|.
    CFG(const std::string& file, Target& target);
    
    CFG(const CFG&) = delete;
    CFG& operator = (const CFG&) = delete;
    
    ~CFG();

    /// Returns the input file that this control flow graph represents.
    const std::string& get_file() const { return m_file; }
    std::string& get_file() { return m_file; }

    /// Set the file that this graph represents to |file|.
    void set_file(std::string& file) { m_file = file; }

    /// Returns the target of this control flow graph.
    const Target& get_target() const { return m_target; }
    Target& get_target() { return m_target; }

    /// Returns a list of all the structure types in this graph, in order of
    /// creation.
    std::vector<StructType*> structs() const;

    /// Returns a list of all functions in this graph, in order of addition.
    std::vector<Global*> globals() const;

    /// Returns the global in this graph with the provided name, if it exists, 
    /// and null otherwise. 
    const Global* get_global(const std::string& name) const;
    Global* get_global(const std::string& name) {
        return const_cast<Global*>(
            static_cast<const CFG*>(this)->get_global(name));
    }

    /// Add |glb| to this graph. Fails if there is any existing top-level value 
    /// with the same name.
    void add_global(Global* glb);

    /// Remove |glb| if it exists in this graph.
    void remove_global(Global* glb);

    /// Returns a list of all functions in this graph, in order of addition.
    std::vector<Function*> functions() const;

    /// Returns the function in this graph with the provided name if it exists, 
    /// and null otherwise.
    const Function* get_function(const std::string& name) const;
    Function* get_function(const std::string& name) {
        return const_cast<Function*>(
            static_cast<const CFG*>(this)->get_function(name));
    }

    /// Add |fn| to this graph. Fails if there is any existing top-level value 
    /// with the same name.
    void add_function(Function* fn);

    /// Remove |fn| if it exists in this graph.
    void remove_function(Function* fn);

    /// Return a new unique definition id to create an instruction with.
    uint32_t get_def_id() { return m_def_id++; }

    /// Print this graph in a reproducible plaintext format to the output
    /// stream |os|.
    void print(std::ostream& os) const;
};

} // namespace spbe

#endif // SPBE_CFG_H_
