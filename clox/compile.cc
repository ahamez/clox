#include <cassert>
#include <functional>

#include <fmt/core.h>
#include <magic_enum.hpp>

#include "clox/compile.hh"
#include "clox/memory.hh"

namespace clox {

using namespace clox::detail;

// ---------------------------------------------------------------------------------------------- //

namespace {

template<typename Opcode>
void
emit(Chunk& chunk, std::size_t line, Opcode&& op)
{
  chunk.code->add_opcode(std::forward<Opcode>(op), line);
}

template<typename Opcode, typename... Opcodes>
void
emit(Chunk& chunk, std::size_t line, Opcode&& op, Opcodes&&... ops)
{
  chunk.code->add_opcode(std::forward<Opcode>(op), line);
  emit(chunk, line, std::forward<Opcodes>(ops)...);
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

  error_msg_ << fmt::format("[line {:d}]", token.line);
  if (token.type == TokenType::eof)
  {
    error_msg_ << " at end";
  }
  else if (token.type == TokenType::error)
  {
    // Nothing.
  }
  else
  {
    error_msg_ << fmt::format(" at {} ({})", token.token, magic_enum::enum_name(token.type));
  }

  error_msg_ << fmt::format(": {}\n", msg);

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
Compile::expression(Chunk& chunk)
{
  parse_precedence(chunk, Precedence::assignement);
}

void
Compile::literal(Chunk& chunk, CanAssign) // NOLINT(readability-make-member-function-const)
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
Compile::number(Chunk& chunk, CanAssign) // NOLINT(readability-make-member-function-const)
{
  const auto value = std::stod(previous_.token.data());
  emit(chunk, previous_.line, OpConstant{chunk.code->add_constant(value)});
}

void
Compile::grouping(Chunk& chunk, CanAssign)
{
  expression(chunk);
  consume(TokenType::right_paren, "Expect ')' after expression.");
}

void
Compile::unary(Chunk& chunk, CanAssign)
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
Compile::binary(Chunk& chunk, CanAssign)
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
Compile::string(Chunk& chunk, CanAssign) // NOLINT(readability-make-member-function-const)
{
  const auto* obj = chunk.memory->make_string(std::string{previous_.token});
  emit(chunk, previous_.line, OpConstant{chunk.code->add_constant(obj)});
}

void
Compile::declaration(Chunk& chunk)
{
  if (match(TokenType::var))
  {
    var_declaration(chunk);
  }
  else
  {
    statement(chunk);
  }

  if (panic_mode_)
  {
    synchronize();
  }
}

void
Compile::statement(Chunk& chunk)
{
  if (match(TokenType::print))
  {
    print_statement(chunk);
  }
  else
  {
    expression_statement(chunk);
  }
}

void
Compile::variable(clox::Chunk& chunk, CanAssign can_assign)
{
  named_variable(chunk, previous_, can_assign);
}

void
Compile::named_variable(Chunk& chunk,
                        Token token,
                        CanAssign can_assign) // NOLINT(readability-make-member-function-const)
{
  const auto var_name = std::string{token.token};
  const auto index = chunk.memory->maybe_add_global_variable(var_name);

  if (can_assign == CanAssign::yes and match(TokenType::equal))
  {
    expression(chunk);
    emit(chunk, previous_.line, OpSetGlobal{index});
  }
  else
  {
    emit(chunk, previous_.line, OpGetGlobalVar{index});
  }
}

// ---------------------------------------------------------------------------------------------- //

void
Compile::print_statement(Chunk& chunk)
{
  expression(chunk);
  consume(TokenType::semicolon, "Expect ';' after expression.");
  emit(chunk, previous_.line, OpPrint{});
}

void
Compile::expression_statement(Chunk& chunk)
{
  expression(chunk);
  consume(TokenType::semicolon, "Expect ';' after expression.");
  emit(chunk, previous_.line, OpPop{});
}

void
Compile::var_declaration(Chunk& chunk)
{
  const auto var_index = parse_variable(chunk, "Expect variable name");

  if (match(TokenType::equal))
  {
    expression(chunk);
  }
  else
  {
    emit(chunk, previous_.line, OpNil{});
  }

  consume(TokenType::semicolon, "Expect ';' after variable declaration");

  emit(chunk, previous_.line, OpDefineGlobalVar{var_index});
}

GlobalVariableIndex
Compile::parse_variable(Chunk& chunk, const std::string& error_msg)
{
  consume(TokenType::identifier, error_msg);
  const auto var_name = std::string{previous_.token};

  return chunk.memory->maybe_add_global_variable(var_name);
}

// ---------------------------------------------------------------------------------------------- //

constexpr const ParseRule&
Compile::get_rule(TokenType token_type)
{
  assert(magic_enum::enum_index(token_type).has_value());
  return parser_rules_[magic_enum::enum_index(token_type).value()];
}

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

  const auto can_assign =
    precedence <= Precedence::assignement ? detail::CanAssign::yes : detail::CanAssign::no;
  std::invoke(prefix_rule, this, chunk, can_assign);

  while (precedence <= get_rule(current_.type).precedence)
  {
    advance();
    const auto& infix_rule = get_rule(previous_.type).infix;
    std::invoke(infix_rule, this, chunk, can_assign);
  }
}

// ---------------------------------------------------------------------------------------------- //

Compile::CompileResut
Compile::operator()(std::unique_ptr<Memory>&& memory)
{
  auto chunk = Chunk{std::make_unique<Code>(), std::move(memory)};

  advance();

  while (not match(TokenType::eof))
  {
    declaration(chunk);
  }

  emit(chunk, previous_.line, OpReturn{});

  if (had_error_)
  {
    return CompileResut::error(std::make_tuple(std::move(chunk.memory), error_msg_.str()));
  }
  else
  {
    return CompileResut::ok(std::move(chunk));
  }
}

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
