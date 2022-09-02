#pragma once

#include <array>
#include <string>

#include <magic_enum.hpp>

#include "clox/chunk.hh"
#include "clox/expected.hh"
#include "clox/scanner.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

// Forward declaration to declare ParseFn.
class Compile;

namespace detail {

using ParseFn = void (Compile::*)(Chunk&);

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

} // detail::anonymous

// ---------------------------------------------------------------------------------------------- //

class Compile
{
public:
  explicit Compile(Scanner&&);

  Expected<Chunk, std::string> operator()();

private:
  void advance();
  void consume(TokenType, const std::string&);

  void expression(Chunk&);
  void binary(Chunk&);
  void grouping(Chunk&);
  void literal(Chunk&);
  void number(Chunk&);
  void unary(Chunk&);
  void string(Chunk&);

  [[nodiscard]] static constexpr const detail::ParseRule& get_rule(TokenType);
  void parse_precedence(Chunk&, detail::Precedence);

  void error_at_current(const std::string&);
  void error_at_previous(const std::string&);
  void error_at(const Token& token, const std::string& msg);

private:
  Scanner scanner_;
  Token current_{};
  Token previous_{};
  bool had_error_{false};
  bool panic_mode_{false};

  using Rule = detail::Rule;
  using Precedence = detail::Precedence;
  static constexpr detail::ParserRules parser_rules_ =
    make_rules(Rule{TokenType::left_paren, {&Compile::grouping, nullptr, Precedence::none}},
               Rule{TokenType::right_paren, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::left_brace, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::right_brace, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::comma, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::dot, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::minus, {&Compile::unary, &Compile::binary, Precedence::term}},
               Rule{TokenType::plus, {nullptr, &Compile::binary, Precedence::term}},
               Rule{TokenType::semicolon, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::slash, {nullptr, &Compile::binary, Precedence::factor}},
               Rule{TokenType::star, {nullptr, &Compile::binary, Precedence::factor}},
               Rule{TokenType::bang, {&Compile::unary, nullptr, Precedence::none}},
               Rule{TokenType::bang_equal, {nullptr, &Compile::binary, Precedence::equality}},
               Rule{TokenType::equal, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::equal_equal, {nullptr, &Compile::binary, Precedence::equality}},
               Rule{TokenType::greater, {nullptr, &Compile::binary, Precedence::comparison}},
               Rule{TokenType::greater_equal, {nullptr, &Compile::binary, Precedence::comparison}},
               Rule{TokenType::less, {nullptr, &Compile::binary, Precedence::comparison}},
               Rule{TokenType::less_equal, {nullptr, &Compile::binary, Precedence::comparison}},
               Rule{TokenType::identifier, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::string, {&Compile::string, nullptr, Precedence::none}},
               Rule{TokenType::number, {&Compile::number, nullptr, Precedence::none}},
               Rule{TokenType::and_, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::class_, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::else_, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::false_, {&Compile::literal, nullptr, Precedence::none}},
               Rule{TokenType::for_, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::fun, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::if_, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::nil, {&Compile::literal, nullptr, Precedence::none}},
               Rule{TokenType::or_, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::print, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::return_, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::super, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::this_, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::true_, {&Compile::literal, nullptr, Precedence::none}},
               Rule{TokenType::var, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::while_, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::error, {nullptr, nullptr, Precedence::none}},
               Rule{TokenType::eof, {nullptr, nullptr, Precedence::none}});
};

// ---------------------------------------------------------------------------------------------- //

} // clox
