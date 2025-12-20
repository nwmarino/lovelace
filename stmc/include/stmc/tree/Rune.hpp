//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef STATIM_RUNE_H_
#define STATIM_RUNE_H_

#include <cstdint>
#include <vector>

namespace stm {

using std::vector;

class Expr;
class Rune;

using RuneVec = vector<Rune*>;

/// Represents a rune in the AST.
class Rune final {
public:
    using ArgVec = vector<Expr*>;

    /// The different kinds of runes.
    enum Kind : uint32_t {
        Public,
        Private,
    };

private:
    const Kind m_kind;
    ArgVec m_args;

public:
    Rune(Kind kind, const ArgVec& args)
      : m_kind(kind), m_args(args) {}

    ~Rune();
        
    Rune(const Rune&) = delete;
    void operator=(const Rune&) = delete;

    Rune(Rune&&) noexcept = delete;
    void operator=(Rune&&) noexcept = delete;

    Kind get_kind() const { return m_kind; }

    uint32_t num_args() const { return m_args.size(); }
    bool has_args() const { return !m_args.empty(); }

    void set_args(const ArgVec& args) { m_args = args; }
    const ArgVec& get_args() const { return m_args; }
    ArgVec& get_args() { return m_args; }
};

} // namespace stm

#endif // STATIM_RUNE_H_
