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
emit(CompileContext& cxt, std::size_t line, Opcode&& op)
{
  cxt.chunk.code->add_opcode(std::forward<Opcode>(op), line);
}

template<typename Opcode, typename... Opcodes>
void
emit(CompileContext& cxt, std::size_t line, Opcode&& op, Opcodes&&... ops)
{
  cxt.chunk.code->add_opcode(std::forward<Opcode>(op), line);
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
Compile::expression(CompileContext& cxt)
{
  parse_precedence(cxt, Precedence::assignement);
}

void
Compile::literal(CompileContext& cxt, CanAssign) // NOLINT(readability-make-member-function-const)
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
Compile::number(CompileContext& cxt, CanAssign) // NOLINT(readability-make-member-function-const)
{
  const auto value = std::stod(previous_.token.data());
  emit(cxt, previous_.line, OpConstant{cxt.chunk.code->add_constant(value)});
}

void
Compile::grouping(CompileContext& cxt, CanAssign)
{
  expression(cxt);
  consume(TokenType::right_paren, "Expect ')' after expression.");
}

void
Compile::unary(CompileContext& cxt, CanAssign)
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
Compile::binary(CompileContext& cxt, CanAssign)
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
Compile::string(CompileContext& cxt, CanAssign) // NOLINT(readability-make-member-function-const)
{
  const auto* obj = cxt.chunk.memory->make_string(std::string{previous_.token});
  emit(cxt, previous_.line, OpConstant{cxt.chunk.code->add_constant(obj)});
}

void
Compile::declaration(CompileContext& cxt)
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
Compile::statement(CompileContext& cxt)
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
Compile::variable(CompileContext& cxt, CanAssign can_assign)
{
  named_variable(cxt, previous_, can_assign);
}

void
Compile::named_variable(CompileContext& cxt,
                        Token token,
                        CanAssign can_assign) // NOLINT(readability-make-member-function-const)
{
  const auto var_name = std::string{token.token};
  const auto index = cxt.chunk.memory->maybe_add_global_variable(var_name);

  if (can_assign == CanAssign::yes and match(TokenType::equal))
  {
    expression(cxt);
    emit(cxt, previous_.line, OpSetGlobal{index});
  }
  else
  {
    emit(cxt, previous_.line, OpGetGlobalVar{index});
  }
}

// ---------------------------------------------------------------------------------------------- //

void
Compile::print_statement(CompileContext& cxt)
{
  expression(cxt);
  consume(TokenType::semicolon, "Expect ';' after expression.");
  emit(cxt, previous_.line, OpPrint{});
}

void
Compile::expression_statement(CompileContext& cxt)
{
  expression(cxt);
  consume(TokenType::semicolon, "Expect ';' after expression.");
  emit(cxt, previous_.line, OpPop<1>{});
}

void
Compile::var_declaration(CompileContext& cxt)
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
Compile::parse_variable(CompileContext& cxt, const std::string& error_msg)
{
  consume(TokenType::identifier, error_msg);
  const auto var_name = std::string{previous_.token};

  return cxt.chunk.memory->maybe_add_global_variable(var_name);
}

// ---------------------------------------------------------------------------------------------- //

constexpr const ParseRule&
Compile::get_rule(TokenType token_type)
{
  // `rules` has the correct size by definition.
  assert(magic_enum::enum_index(token_type).has_value());
  // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
  return parser_rules_[magic_enum::enum_index(token_type).value()];
}

void
Compile::parse_precedence(CompileContext& cxt, Precedence precedence)
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
  std::invoke(prefix_rule, this, cxt, can_assign);

  while (precedence <= get_rule(current_.type).precedence)
  {
    advance();
    const auto& infix_rule = get_rule(previous_.type).infix;
    std::invoke(infix_rule, this, cxt, can_assign);
  }
}

// ---------------------------------------------------------------------------------------------- //

Compile::CompileResult
Compile::operator()(std::shared_ptr<Memory> memory)
{
  auto code = std::make_shared<Code>();
  auto cxt = CompileContext{Chunk{code, memory}};

  advance();

  while (not match(TokenType::eof))
  {
    declaration(cxt);
  }

  emit(cxt, previous_.line, OpReturn{});

  if (had_error_)
  {
    return boost::leaf::new_error(memory, error_msg_.str());
  }
  else
  {
    return cxt.chunk;
  }
}

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
