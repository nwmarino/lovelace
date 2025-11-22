//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "ast/Context.hpp"
#include "ast/Type.hpp"

using namespace scc;

Context::Context() {
    using TK = Context::TypeKind;

    m_prims[TK::Void] = std::unique_ptr<VoidType>(new VoidType());
    m_prims[TK::UChar] = std::unique_ptr<IntegerType>(new IntegerType(8, false));
    m_prims[TK::Char] = std::unique_ptr<IntegerType>(new IntegerType(8, true));
    m_prims[TK::UShort] = std::unique_ptr<IntegerType>(new IntegerType(16, false));
    m_prims[TK::Short] = std::unique_ptr<IntegerType>(new IntegerType(16, true));
    m_prims[TK::UInt] = std::unique_ptr<IntegerType>(new IntegerType(32, false));
    m_prims[TK::Int] = std::unique_ptr<IntegerType>(new IntegerType(32, true));
    m_prims[TK::ULong] = std::unique_ptr<IntegerType>(new IntegerType(64, false));
    m_prims[TK::Long] = std::unique_ptr<IntegerType>(new IntegerType(64, true));
    m_prims[TK::ULongLong] = std::unique_ptr<IntegerType>(new IntegerType(64, false));
    m_prims[TK::LongLong] = std::unique_ptr<IntegerType>(new IntegerType(64, true));
    m_prims[TK::Float] = std::unique_ptr<FPType>(new FPType(32));
    m_prims[TK::Double] = std::unique_ptr<FPType>(new FPType(64));    
}
