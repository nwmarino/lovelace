//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_RUNE_H_
#define LOVELACE_RUNE_H_

//
//  This header file defines the Rune type, which is a representation of the
//  language construct of the same name that gives way to many of the meta-
//  programming abilities in the language.
//

#include <cstdint>
#include <vector>

namespace lace {

class Expr;
class Rune;

using Runes = std::vector<Rune*>;

/// Represents a rune in the AST.
class Rune final {
public:
    using Args = std::vector<Expr*>;

    /// The different kinds of runes.
    enum Kind : uint32_t {
        Abort,
        Intrinsic,
        Public,
        Private,
        Unreachable,
    };

private:
    const Kind m_kind;
    Args m_args;

public:
    Rune(Kind kind, const Args& args = {}) : m_kind(kind), m_args(args) {}

    ~Rune();
        
    Rune(const Rune&) = delete;
    void operator=(const Rune&) = delete;

    Rune(Rune&&) noexcept = delete;
    void operator=(Rune&&) noexcept = delete;

    Kind get_kind() const { return m_kind; }

    void set_args(const Args& args) { m_args = args; }
    const Args& get_args() const { return m_args; }
    Args& get_args() { return m_args; }

    uint32_t num_args() const { return m_args.size(); }
    bool has_args() const { return !m_args.empty(); }
};

} // namespace lace

#endif // LOVELACE_RUNE_H_
