#include <cassert>
#include <functional>
#include <iostream>

#include <fmt/core.h>
#include <magic_enum.hpp>

#include "clox/compile.hh"

// NOLINTBEGIN(readability-make-member-function-const)

namespace clox {

using namespace clox::detail;

// ---------------------------------------------------------------------------------------------- //

namespace {

template<typename Opcode>
void
emit(Chunk& cxt, std::size_t line, Opcode&& op)
{
  cxt.code.add_opcode(std::forward<Opcode>(op), line);
}

template<typename Opcode, typename... Opcodes>
void
emit(Chunk& cxt, std::size_t line, Opcode&& op, Opcodes&&... ops)
{
  cxt.code.add_opcode(std::forward<Opcode>(op), line);
  emit(cxt, line, std::forward<Opcodes>(ops)...);
}

} // namespace

// ---------------------------------------------------------------------------------------------- //

Compile::Compile(Scanner&& scanner)
  : scanner_{std::move(scanner)}
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

bool
Compile::match(TokenType tokenType)
{
  if (not check(tokenType))
  {
    return false;
  }
  else
  {
    advance();
    return true;
  }
}
bool
Compile::check(TokenType token_type) const noexcept
{
  return current_.type == token_type;
}

void
Compile::synchronize()
{
  panic_mode_ = false;

  while (current_.type != TokenType::eof)
  {
    if (previous_.type == TokenType::semicolon)
    {
      return;
    }
    else
    {
      // NOLINTBEGIN(bugprone-branch-clone)
      switch (current_.type)
      {
        case TokenType::class_:
          [[fallthrough]];
        case TokenType::fun:
          [[fallthrough]];
        case TokenType::var:
          [[fallthrough]];
        case TokenType::for_:
          [[fallthrough]];
        case TokenType::if_:
          [[fallthrough]];
        case TokenType::while_:
          [[fallthrough]];
        case TokenType::print:
          [[fallthrough]];
        case TokenType::return_:
          return;

        default:
          // Do nothing
          ;
      }
      // NOLINTEND(bugprone-branch-clone)
    }

    advance();
  }
}

// ---------------------------------------------------------------------------------------------- //

void
Compile::expression(Chunk& cxt)
{
  parse_precedence(cxt, Precedence::assignement);
}

void
Compile::literal(Chunk& cxt)
{
  switch (previous_.type)
  {
    case TokenType::false_:
      emit(cxt, previous_.line, OpFalse{});
      break;
    case TokenType::true_:
      emit(cxt, previous_.line, OpTrue{});
      break;
    case TokenType::nil:
      emit(cxt, previous_.line, OpNil{});
      break;
    default:
      __builtin_unreachable();
  }
}

void
Compile::number(Chunk& cxt)
{
  const auto value = std::stod(previous_.token.data());
  emit(cxt, previous_.line, OpConstant{cxt.code.add_constant(value)});
}

void
Compile::grouping(Chunk& cxt)
{
  expression(cxt);
  consume(TokenType::right_paren, "Expect ')' after expression.");
}

void
Compile::unary(Chunk& cxt)
{
  const auto operator_type = previous_.type;

  // Compile the operand.
  parse_precedence(cxt, Precedence::unary);

  switch (operator_type)
  {
    case TokenType::bang:
      emit(cxt, previous_.line, OpNot{});
      break;
    case TokenType::minus:
      emit(cxt, previous_.line, OpNegate{});
      break;
    default:
      __builtin_unreachable();
  }
}

void
Compile::binary(Chunk& cxt)
{
  const auto operator_type = previous_.type;
  const auto rule = get_rule(operator_type);
  parse_precedence(cxt, rule.precedence);

  switch (operator_type)
  {
    case TokenType::plus:
      emit(cxt, previous_.line, OpAdd{});
      break;
    case TokenType::minus:
      emit(cxt, previous_.line, OpSubtract{});
      break;
    case TokenType::star:
      emit(cxt, previous_.line, OpMultiply{});
      break;
    case TokenType::slash:
      emit(cxt, previous_.line, OpDivide{});
      break;
    case TokenType::bang_equal:
      emit(cxt, previous_.line, OpEqual{}, OpNot{});
      break;
    case TokenType::equal_equal:
      emit(cxt, previous_.line, OpEqual{});
      break;
    case TokenType::greater:
      emit(cxt, previous_.line, OpGreater{});
      break;
    case TokenType::greater_equal:
      emit(cxt, previous_.line, OpLess{}, OpNot{});
      break;
    case TokenType::less:
      emit(cxt, previous_.line, OpLess{});
      break;
    case TokenType::less_equal:
      emit(cxt, previous_.line, OpGreater{}, OpNot{});
      break;
    default:
      __builtin_unreachable();
  }
}

void
Compile::string(Chunk& cxt)
{
  const auto* obj = cxt.code.memory().make_string(std::string{previous_.token});
  emit(cxt, previous_.line, OpConstant{cxt.code.add_constant(obj)});
}

void
Compile::declaration(Chunk& cxt)
{
  if (match(TokenType::var))
  {
    var_declaration(cxt);
  }
  else
  {
    statement(cxt);
  }

  if (panic_mode_)
  {
    synchronize();
  }
}

void
Compile::statement(Chunk& cxt)
{
  if (match(TokenType::print))
  {
    print_statement(cxt);
  }
  else
  {
    expression_statement(cxt);
  }
}

void
Compile::variable(clox::Chunk& cxt)
{
  named_variable(cxt, previous_);
}

void
Compile::named_variable(Chunk& cxt, Token token)
{
  const auto var_name = std::string{token.token};
  const auto index = cxt.code_cxt.maybe_add_global_variable(var_name);

  emit(cxt, previous_.line, OpGetGlobalVar{index});
}

// ---------------------------------------------------------------------------------------------- //

void
Compile::print_statement(Chunk& cxt)
{
  expression(cxt);
  consume(TokenType::semicolon, "Expect ';' after expression.");
  emit(cxt, previous_.line, OpPrint{});
}

void
Compile::expression_statement(Chunk& cxt)
{
  expression(cxt);
  consume(TokenType::semicolon, "Expect ';' after expression.");
  emit(cxt, previous_.line, OpPop{});
}

void
Compile::var_declaration(Chunk& cxt)
{
  const auto var_index = parse_variable(cxt, "Expect variable name");

  if (match(TokenType::equal))
  {
    expression(cxt);
  }
  else
  {
    emit(cxt, previous_.line, OpNil{});
  }

  consume(TokenType::semicolon, "Expect ';' after variable declaration");

  emit(cxt, previous_.line, OpDefineGlobalVar{var_index});
}

GlobalVariableIndex
Compile::parse_variable(Chunk& cxt, const std::string& error_msg)
{
  consume(TokenType::identifier, error_msg);
  const auto var_name = std::string{previous_.token};

  return cxt.code_cxt.maybe_add_global_variable(var_name);
}

// ---------------------------------------------------------------------------------------------- //

constexpr const ParseRule&
Compile::get_rule(TokenType token_type)
{
  assert(magic_enum::enum_index(token_type).has_value());
  return parser_rules_[magic_enum::enum_index(token_type).value()];
}

void
Compile::parse_precedence(Chunk& cxt, Precedence precedence)
{
  advance();

  const auto& prefix_rule = get_rule(previous_.type).prefix;
  if (prefix_rule == nullptr)
  {
    error_at_previous("Expect expression");
    return;
  }
  std::invoke(prefix_rule, this, cxt);

  while (precedence <= get_rule(current_.type).precedence)
  {
    advance();
    const auto& infix_rule = get_rule(previous_.type).infix;
    std::invoke(infix_rule, this, cxt);
  }
}

// ---------------------------------------------------------------------------------------------- //

Expected<clox::Code, std::string>
Compile::operator()(CodeContext& chunk_cxt)
{
  auto chunk = Code{};
  auto cxt = Chunk{chunk, chunk_cxt};

  advance();

  while (not match(TokenType::eof))
  {
    declaration(cxt);
  }

  emit(cxt, previous_.line, OpReturn{});

  if (had_error_)
  {
    return Expected<clox::Code, std::string>::error("Error");
  }
  else
  {
    return Expected<clox::Code, std::string>::ok(std::move(chunk));
  }
}

// ---------------------------------------------------------------------------------------------- //

} // namespace clox

// NOLINTEND(readability-make-member-function-const)
