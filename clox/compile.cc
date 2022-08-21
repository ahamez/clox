#include <cassert>
#include <functional>
#include <iostream>

#include <fmt/core.h>
#include <magic_enum.hpp>

#include "clox/compile.hh"

namespace clox {

using namespace clox::detail;

// ---------------------------------------------------------------------------------------------- //

Compile::Compile(Scanner&& scanner)
  : scanner_{std::move(scanner)}
  , current_{TokenType::error}
  , previous_{TokenType::error}
  , had_error_{false}
  , panic_mode_{false}
{}

// ---------------------------------------------------------------------------------------------- //

void
Compile::error_at_current(const std::string& msg)
{
  error_at(current_, msg);
}

// ---------------------------------------------------------------------------------------------- //

void
Compile::error_at_previous(const std::string& msg)
{
  error_at(previous_, msg);
}

// ---------------------------------------------------------------------------------------------- //

void
Compile::error_at(const Token& token, const std::string& msg)
{
  if (panic_mode_)
  {
    return;
  }
  panic_mode_ = true;

  std::cerr << fmt::format("[line {:d}]", token.line);
  if (token.type == TokenType::eof)
  {
    std::cerr << " at end";
  }
  else if (token.type == TokenType::error)
  {
    // Nothing.
  }
  else
  {
    std::cerr << fmt::format(" at {} ({})", token.token, magic_enum::enum_name(token.type));
  }

  std::cerr << fmt::format(": {}\n", msg);

  had_error_ = true;
}

// ---------------------------------------------------------------------------------------------- //

void
Compile::advance()
{
  previous_ = current_;

  while (true)
  {
    current_ = scanner_.next_token();

    if (current_.type != TokenType::error)
    {
      break;
    }

    error_at_current(std::string{current_.token});
  }
}

// ---------------------------------------------------------------------------------------------- //

void
Compile::literal()
{
  switch (previous_.type)
  {
    case TokenType::false_:
      emit(OpFalse{});
      break;
    case TokenType::true_:
      emit(OpTrue{});
      break;
    case TokenType::nil:
      emit(OpNil{});
      break;
    default:
      __builtin_unreachable();
  }
}

void
Compile::expression()
{
  parse_precedence(Precedence::assignement);
}

void
Compile::number()
{
  const auto value = std::stod(previous_.token.data());
  emit(OpConstant{chunk_.add_value(value)});
}

void
Compile::grouping()
{
  expression();
  consume(TokenType::right_paren, "Expect ')' after expression.");
}

void
Compile::unary()
{
  const auto operator_type = previous_.type;

  // Compile the operand.
  parse_precedence(Precedence::unary);

  switch (operator_type)
  {
    case TokenType::bang:
      emit(OpNot{});
      break;
    case TokenType::minus:
      emit(OpNegate{});
      break;
    default:
      __builtin_unreachable();
  }
}

void
Compile::binary()
{
  const auto operator_type = previous_.type;
  const auto rule = get_rule(operator_type);
  parse_precedence(rule.precedence);

  switch (operator_type)
  {
    case TokenType::plus:
      emit(OpAdd{});
      break;
    case TokenType::minus:
      emit(OpSubtract{});
      break;
    case TokenType::star:
      emit(OpMultiply{});
      break;
    case TokenType::slash:
      emit(OpDivide{});
      break;
    case TokenType::bang_equal:
      emit(OpEqual{}, OpNot{});
      break;
    case TokenType::equal_equal:
      emit(OpEqual{});
      break;
    case TokenType::greater:
      emit(OpGreater{});
      break;
    case TokenType::greater_equal:
      emit(OpLess{}, OpNot{});
      break;
    case TokenType::less:
      emit(OpLess{});
      break;
    case TokenType::less_equal:
      emit(OpGreater{}, OpNot{});
      break;
    default:
      __builtin_unreachable();
  }
}

constexpr const ParseRule&
Compile::get_rule(TokenType token_type)
{
  assert(magic_enum::enum_index(token_type).has_value());
  return parser_rules_[magic_enum::enum_index(token_type).value()];
}

// ---------------------------------------------------------------------------------------------- //

void
Compile::consume(TokenType type, const std::string& msg)
{
  if (current_.type == type)
  {
    advance();
    return;
  }

  error_at_current(msg);
}

// ---------------------------------------------------------------------------------------------- //

void
Compile::parse_precedence(Precedence precedence)
{
  advance();

  const auto& prefix_rule = get_rule(previous_.type).prefix;
  if (prefix_rule == nullptr)
  {
    error_at_previous("Expect expression");
    return;
  }
  std::invoke(prefix_rule, this);

  while (precedence <= get_rule(current_.type).precedence)
  {
    advance();
    const auto& infix_rule = get_rule(previous_.type).infix;
    std::invoke(infix_rule, this);
  }
}

// ---------------------------------------------------------------------------------------------- //

Expected<clox::Chunk, std::string>
Compile::operator()()
{
  advance();
  expression();
  consume(TokenType::eof, "Expected end of expression");
  emit(OpReturn{});

  if (had_error_)
  {
    return Expected<clox::Chunk, std::string>::error("Error");
  }
  else
  {
    return Expected<clox::Chunk, std::string>::ok(chunk_);
  }
}

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
