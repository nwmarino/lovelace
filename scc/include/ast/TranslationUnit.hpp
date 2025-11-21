#ifndef SCC_TRANSLATION_UNIT_H_
#define SCC_TRANSLATION_UNIT_H_

#include "Decl.hpp"

#include <memory>
#include <vector>

namespace scc {

class TranslationUnit final {
    std::unique_ptr<Scope> m_scope;
    std::vector<std::unique_ptr<Decl>> m_decls;
};

} // namespace scc

#endif // SCC_TRANSLATION_UNIT_H_
