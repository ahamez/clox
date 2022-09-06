#pragma once

#include <array>
#include <sstream>
#include <string>

#include <boost/leaf.hpp>

#include "clox/chunk.hh"
#include "clox/detail/compile.hh"
#include "clox/scanner.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

class Compile
{
public:
  using CompileResult = boost::leaf::result<Chunk>;

public:
  explicit Compile(Scanner&&);

  CompileResult operator()(std::shared_ptr<Memory>);

private:
  void advance();
  void consume(TokenType, const std::string& error_msg);
  [[nodiscard]] bool match(TokenType);
  [[nodiscard]] bool check(TokenType) const noexcept;
  void synchronize();

  void expression(Chunk&);

  void binary(Chunk&, detail::CanAssign);
  void grouping(Chunk&, detail::CanAssign);
  void literal(Chunk&, detail::CanAssign);
  void number(Chunk&, detail::CanAssign);
  void unary(Chunk&, detail::CanAssign);
  void string(Chunk&, detail::CanAssign);
  void variable(Chunk&, detail::CanAssign);
  void named_variable(Chunk&, Token, detail::CanAssign);

  void declaration(Chunk&);
  void statement(Chunk&);

  void print_statement(Chunk&);
  void expression_statement(Chunk&);

  void var_declaration(Chunk&);
  detail::GlobalVariableIndex parse_variable(Chunk&, const std::string& error_msg);

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
  std::stringstream error_msg_{};

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
               Rule{TokenType::identifier, {&Compile::variable, nullptr, Precedence::none}},
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

} // namespace clox
