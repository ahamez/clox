#include <cassert>
#include <functional>
#include <iostream>

#include <fmt/core.h>
#include <magic_enum.hpp>

#include "clox/compile.hh"

namespace clox {

using namespace clox::detail;

// ---------------------------------------------------------------------------------------------- //

namespace {

template<typename Opcode>
void
emit(Chunk& chunk, std::size_t line, Opcode&& op)
{
  chunk.add_opcode(std::forward<Opcode>(op), line);
}

template<typename Opcode, typename... Opcodes>
void
emit(Chunk& chunk, std::size_t line, Opcode&& op, Opcodes&&... ops)
{
  chunk.add_opcode(std::forward<Opcode>(op), line);
  emit(chunk, line, std::forward<Opcodes>(ops)...);
}

} // namespace anonymous

// ---------------------------------------------------------------------------------------------- //

Compile::Compile(Scanner&& scanner)
  : scanner_{std::move(scanner)}
  , current_{}
  , previous_{}
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
Compile::expression(Chunk& chunk)
{
  parse_precedence(chunk, Precedence::assignement);
}

void
Compile::literal(Chunk& chunk)
{
  switch (previous_.type)
  {
    case TokenType::false_:
      emit(chunk, previous_.line, OpFalse{});
      break;
    case TokenType::true_:
      emit(chunk, previous_.line, OpTrue{});
      break;
    case TokenType::nil:
      emit(chunk, previous_.line, OpNil{});
      break;
    default:
      __builtin_unreachable();
  }
}

void
Compile::number(Chunk& chunk)
{
  const auto value = std::stod(previous_.token.data());
  emit(chunk, previous_.line, OpConstant{chunk.add_value(value)});
}

void
Compile::grouping(Chunk& chunk)
{
  expression(chunk);
  consume(TokenType::right_paren, "Expect ')' after expression.");
}

void
Compile::unary(Chunk& chunk)
{
  const auto operator_type = previous_.type;

  // Compile the operand.
  parse_precedence(chunk, Precedence::unary);

  switch (operator_type)
  {
    case TokenType::bang:
      emit(chunk, previous_.line, OpNot{});
      break;
    case TokenType::minus:
      emit(chunk, previous_.line, OpNegate{});
      break;
    default:
      __builtin_unreachable();
  }
}

void
Compile::binary(Chunk& chunk)
{
  const auto operator_type = previous_.type;
  const auto rule = get_rule(operator_type);
  parse_precedence(chunk, rule.precedence);

  switch (operator_type)
  {
    case TokenType::plus:
      emit(chunk, previous_.line, OpAdd{});
      break;
    case TokenType::minus:
      emit(chunk, previous_.line, OpSubtract{});
      break;
    case TokenType::star:
      emit(chunk, previous_.line, OpMultiply{});
      break;
    case TokenType::slash:
      emit(chunk, previous_.line, OpDivide{});
      break;
    case TokenType::bang_equal:
      emit(chunk, previous_.line, OpEqual{}, OpNot{});
      break;
    case TokenType::equal_equal:
      emit(chunk, previous_.line, OpEqual{});
      break;
    case TokenType::greater:
      emit(chunk, previous_.line, OpGreater{});
      break;
    case TokenType::greater_equal:
      emit(chunk, previous_.line, OpLess{}, OpNot{});
      break;
    case TokenType::less:
      emit(chunk, previous_.line, OpLess{});
      break;
    case TokenType::less_equal:
      emit(chunk, previous_.line, OpGreater{}, OpNot{});
      break;
    default:
      __builtin_unreachable();
  }
}

void
Compile::string(Chunk& chunk)
{
  auto obj = chunk.memory().make_string(std::string{previous_.token});
  emit(chunk, previous_.line, OpConstant{chunk.add_value(obj)});
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
Compile::parse_precedence(Chunk& chunk, Precedence precedence)
{
  advance();

  const auto& prefix_rule = get_rule(previous_.type).prefix;
  if (prefix_rule == nullptr)
  {
    error_at_previous("Expect expression");
    return;
  }
  std::invoke(prefix_rule, this, chunk);

  while (precedence <= get_rule(current_.type).precedence)
  {
    advance();
    const auto& infix_rule = get_rule(previous_.type).infix;
    std::invoke(infix_rule, this, chunk);
  }
}

// ---------------------------------------------------------------------------------------------- //

Expected<clox::Chunk, std::string>
Compile::operator()()
{
  auto chunk = Chunk{};

  advance();
  expression(chunk);
  consume(TokenType::eof, "Expected end of expression");
  emit(chunk, previous_.line, OpReturn{});

  if (had_error_)
  {
    return Expected<clox::Chunk, std::string>::error("Error");
  }
  else
  {
    return Expected<clox::Chunk, std::string>::ok(std::move(chunk));
  }
}

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
