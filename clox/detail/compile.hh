#pragma once

#include <array>
#include <sstream>
#include <string>

#include <magic_enum.hpp>

#include "clox/chunk.hh"
#include "clox/scanner.hh"

// ---------------------------------------------------------------------------------------------- //

namespace clox {
// Forward declaration to declare ParseFn.
class Compile;
} // namespace clox

// ---------------------------------------------------------------------------------------------- //

namespace clox::detail {

// ---------------------------------------------------------------------------------------------- //

enum class CanAssign
{
  yes,
  no
};

using ParseFn = void (Compile::*)(Chunk&, CanAssign);

enum class Precedence
{
  none,
  assignement,
  or_,
  and_,
  equality,
  comparison,
  term,
  factor,
  unary,
  call,
  primary
};

struct ParseRule
{
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
};

struct Rule
{
  TokenType token_type;
  ParseRule parse_rule;
};

using ParserRules = std::array<ParseRule, magic_enum::enum_count<TokenType>()>;

template<typename... Ts>
constexpr ParserRules
make_rules(Ts... xs)
{
  auto rules = ParserRules{};
  return make_rules_impl(rules, std::forward<Ts>(xs)...);
}

template<typename T>
constexpr ParserRules
make_rules_impl(ParserRules& rules, T x)
{
  rules[magic_enum::enum_index(x.token_type).value()] = x.parse_rule;
  return rules;
}

template<typename T, typename... Ts>
constexpr ParserRules
make_rules_impl(ParserRules& rules, T x, Ts... xs)
{
  rules[magic_enum::enum_index(x.token_type).value()] = x.parse_rule;
  return make_rules_impl<Ts...>(rules, std::forward<Ts>(xs)...);
}

} // namespace clox::detail
