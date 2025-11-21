#ifndef SCC_TYPE_H_
#define SCC_TYPE_H_

#include <cstdint>
#include <memory>
#include <vector>

namespace scc {

class EnumDecl;
class StructDecl;

class Type {
    
};

class VoidType final : public Type {

};

class IntegerType final : public Type {
    uint32_t m_width;
    bool m_signed;
};

class FloatType final : public Type {
    uint32_t m_width;
};

class ArrayType final : public Type {
    std::shared_ptr<Type> m_element;
    uint32_t m_size;
};

class PointerType final : public Type {
    std::shared_ptr<Type> m_pointee;
};

class FunctionType final : public Type {
    std::shared_ptr<Type> m_return;
    std::vector<std::shared_ptr<Type>> m_params;
};

class AliasedType final : public Type {
    std::shared_ptr<Type> m_type;
};

class StructType final : public Type {
    std::vector<std::shared_ptr<Type>> m_fields;
};

class EnumType final : public Type {
    
};

class UnionType final : public Type {

};

} // namespace scc

#endif // SCC_TYPE_H_
