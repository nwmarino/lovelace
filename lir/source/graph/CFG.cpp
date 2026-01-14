//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/graph/CFG.hpp"
#include "lir/graph/Constant.hpp"
#include "lir/graph/Type.hpp"

using namespace lir;

CFG::CFG(const Machine& mach, const std::string& filename) 
  : m_mach(mach), m_filename(filename) {
    m_types.void_type = new VoidType();
    m_types.ints.emplace(1, new IntegerType(1));
    m_types.ints.emplace(8, new IntegerType(8));
    m_types.ints.emplace(16, new IntegerType(16));
    m_types.ints.emplace(32, new IntegerType(32));
    m_types.ints.emplace(64, new IntegerType(64));
    m_types.floats.emplace(32, new FloatType(32));
    m_types.floats.emplace(64, new FloatType(64));

    m_constants.zero = new Integer(0, m_types.ints[1]);
    m_constants.one = new Integer(1, m_types.ints[1]);
}

CFG::~CFG() {
    for (auto& pair : m_globals)
        delete pair.second;

    for (auto& pair : m_functions)
        delete pair.second;

    m_globals.clear();
    m_functions.clear();

    if (m_types.void_type)
        delete m_types.void_type;

    for (auto& pair : m_types.ints)
        delete pair.second;

    for (auto& pair : m_types.floats)
        delete pair.second;

    for (auto& [element, size_pair] : m_types.arrays) {
        for (auto& [size, type] : size_pair)
            delete type;

        size_pair.clear();
    }

    for (auto& pair : m_types.pointers)
        delete pair.second;

    for (auto& pair : m_types.structs)
        delete pair.second;

    for (auto& type : m_types.functions)
        delete type;

    m_types.void_type = nullptr;
    m_types.arrays.clear();
    m_types.floats.clear();
    m_types.functions.clear();
    m_types.ints.clear();
    m_types.pointers.clear();
    m_types.structs.clear();

    if (m_constants.zero)
        delete m_constants.zero;

    if (m_constants.one)
        delete m_constants.one;

    for (auto& pair : m_constants.bytes)
        delete pair.second;

    for (auto& pair : m_constants.shorts)
        delete pair.second;

    for (auto& pair : m_constants.ints) 
        delete pair.second;

    for (auto& pair : m_constants.longs)
        delete pair.second;

    for (auto& pair : m_constants.floats)
        delete pair.second;

    for (auto& pair : m_constants.doubles)
        delete pair.second;

    for (auto& pair : m_constants.nulls)
        delete pair.second;

    for (auto& pair : m_constants.strings)
        delete pair.second;

    for (auto& pair : m_constants.addresses)
        delete pair.second;

    m_constants.zero = nullptr;
    m_constants.one = nullptr;
    m_constants.bytes.clear();
    m_constants.shorts.clear();
    m_constants.ints.clear();
    m_constants.longs.clear();
    m_constants.floats.clear();
    m_constants.doubles.clear();
    m_constants.nulls.clear();
    m_constants.strings.clear();
    m_constants.addresses.clear();
}

std::vector<StructType*> CFG::get_structs() const {
    std::vector<StructType*> structs;
    structs.reserve(m_types.structs.size());
    for (const auto& pair : m_types.structs)
        structs.push_back(pair.second);

    return structs;
}

std::vector<Global*> CFG::get_globals() const {
    std::vector<Global*> globals;
    globals.reserve(m_globals.size());

    for (const auto& pair : m_globals)
        globals.push_back(pair.second);

    return globals;
}

const Global* CFG::get_global(const std::string& name) const {
    auto it = m_globals.find(name);
    if (it != m_globals.end())
        return it->second;

    return nullptr;
}

void CFG::add_global(Global* global) {
    assert(global && "global cannot be null!");
    assert(!get_global(global->get_name()) && 
        !get_function(global->get_name()) && 
        "global has name conflicts with an existing symbol!");

    m_globals.emplace(global->get_name(), global);
    global->set_parent(this);
}

void CFG::remove_global(Global* glb) {
    auto it = m_globals.find(glb->get_name());
    if (it != m_globals.end()) {
        assert(it->second == glb);
        assert(glb->get_parent() == this);

        m_globals.erase(it);
    }
}

std::vector<Function*> CFG::get_functions() const {
    std::vector<Function*> functions;
    functions.reserve(m_functions.size());

    for (const auto& pair : m_functions)
        functions.push_back(pair.second);

    return functions;
}

const Function* CFG::get_function(const std::string& name) const {
    auto it = m_functions.find(name);
    if (it != m_functions.end())
        return it->second;

    return nullptr;
}

void CFG::add_function(Function* function) {
    assert(function && "function cannot be null!");
    assert(!get_global(function->get_name()) && 
        !get_function(function->get_name()) && 
        "function has name conflicts with an existing symbol!");

    m_functions.emplace(function->get_name(), function);
    function->set_parent(this);
}

void CFG::remove_function(Function* function) {
    auto it = m_functions.find(function->get_name());
    if (it != m_functions.end()) {
        assert(it->second == function);
        assert(function->get_parent() == this);

        m_functions.erase(it);
    }
}
