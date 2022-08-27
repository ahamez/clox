#include <algorithm>
#include <exception>
#include <iostream>
#include <iterator>
#include <vector>

#include "clox/vm.hh"

namespace clox {
namespace /* anonymous */ {

using Stack = std::vector<Value>;

// ---------------------------------------------------------------------------------------------- //

void
push(Stack& stack, Value value)
{
  stack.push_back(value);
}

[[nodiscard]] Value
pop(Stack& stack)
{
  const auto value = stack.back();
  stack.pop_back();
  return value;
}

[[nodiscard]] Value
peek(Stack& stack, int distance)
{
  return *(cend(stack) - 1 - distance);
}

// ---------------------------------------------------------------------------------------------- //

struct InterpretReturn : public std::exception
{
  explicit InterpretReturn(InterpretResult r)
    : InterpretReturn{r, {}}
  {}

  explicit InterpretReturn(InterpretResult r, std::string msg)
    : result{r}
    , message{std::move(msg)}
  {}

  [[nodiscard]] const char* what() const noexcept override { return message.data(); }

  InterpretResult result;
  std::string message;
};

// ---------------------------------------------------------------------------------------------- //

struct Interpret
{
  Chunk& chunk;
  Stack& stack;
  const Chunk::code_const_iterator current_ip;

  template<typename Op>
  Chunk::code_const_iterator operator()(OpBinary<Op>)
  {
    if (not peek(stack, 0).is<double>() or not peek(stack, 1).is<double>())
    {
      throw InterpretReturn{InterpretResult::runtime_error, "Operands must be numbers"};
    }
    else
    {
      const auto rhs = pop(stack);
      const auto lhs = stack.back();
      stack.back() = OpBinary<Op>{}(lhs, rhs);

      return std::next(current_ip);
    }
  }

  Chunk::code_const_iterator operator()(OpAdd)
  {
    if (peek(stack, 0).is<double>() and peek(stack, 1).is<double>())
    {
      const auto rhs = pop(stack);
      const auto lhs = stack.back();
      stack.back() = OpAdd{}(lhs, rhs);
      return std::next(current_ip);
    }
    else if (peek(stack, 0).is<const ObjString*>() and peek(stack, 1).is<const ObjString*>())
    {
      const auto rhs = pop(stack);
      const auto lhs = stack.back();

      const auto& lhs_str = lhs.as<const ObjString*>()->str;
      const auto& rhs_str = rhs.as<const ObjString*>()->str;

      stack.back() = chunk.memory().make_string(lhs_str + rhs_str);
      return std::next(current_ip);
    }
    else
    {
      throw InterpretReturn{InterpretResult::runtime_error, "Operands must be numbers or strings"};
    }
  }

  Chunk::code_const_iterator operator()(OpConstant op)
  {
    const auto value = chunk.get_value(op.value_offset);
    push(stack, value);

    return std::next(current_ip);
  }

  Chunk::code_const_iterator operator()(OpEqual)
  {
    const auto rhs = pop(stack);
    const auto lhs = stack.back();
    stack.back() = (rhs == lhs);

    return std::next(current_ip);
  }

  Chunk::code_const_iterator operator()(OpFalse)
  {
    push(stack, false);

    return std::next(current_ip);
  }

  Chunk::code_const_iterator operator()(OpNegate)
  {
    if (not peek(stack, 0).is<double>())
    {
      throw InterpretReturn{InterpretResult::runtime_error, "Operand must be a number"};
    }
    else
    {
      stack.back() = -stack.back().as<double>();

      return std::next(current_ip);
    }
  }

  Chunk::code_const_iterator operator()(OpNil)
  {
    push(stack, Nil{});

    return std::next(current_ip);
  }

  Chunk::code_const_iterator operator()(OpNot)
  {
    push(stack, pop(stack).falsey());

    return std::next(current_ip);
  }

  [[noreturn]] Chunk::code_const_iterator operator()(OpReturn)
  {
    std::cout << pop(stack) << '\n';

    throw InterpretReturn{InterpretResult::ok};
  }

  Chunk::code_const_iterator operator()(OpTrue)
  {
    push(stack, true);

    return std::next(current_ip);
  }

  [[nodiscard]] Chunk::code_const_iterator visit() const
  {
    return std::visit(Interpret{chunk, stack, current_ip}, *current_ip);
  }
};

// NOLINTBEGIN(misc-no-recursion)
template<VM::opt_disassemble Disassemble>
[[nodiscard]] InterpretResult
run(Chunk& chunk,
    Stack& stack,
    Chunk::code_const_iterator current_ip,
    std::ostream& os_disassembling)
{
  try
  {
    if constexpr (Disassemble == VM::opt_disassemble::yes)
    {
      os_disassembling << chunk.disassemble(current_ip) << '\n';
    }
    const auto next_ip = Interpret{chunk, stack, current_ip}.visit();

    return run<Disassemble>(chunk, stack, next_ip, os_disassembling);
  }
  catch (const InterpretReturn& r)
  {
    if (r.result != InterpretResult::ok)
    {
      std::cerr << "line " << chunk.line(current_ip).value_or(0) << ": " << r.message << '\n';
    }
    return r.result;
  }
}
// NOLINTEND(misc-no-recursion)

} // anonymous

// ---------------------------------------------------------------------------------------------- //

VM::VM(VM::opt_disassemble disassemble)
  : disassemble_{disassemble}
{}

// ---------------------------------------------------------------------------------------------- //

InterpretResult
VM::operator()(Chunk& chunk) const
{
  auto stack = Stack{};
  stack.reserve(1024);

  const auto ip = chunk.code_cbegin();

  if (disassemble_ == opt_disassemble::yes)
  {
    return run<opt_disassemble::yes>(chunk, stack, ip, std::cout);
  }
  else
  {
    return run<opt_disassemble::no>(chunk, stack, ip, std::cout);
  }
}

// ---------------------------------------------------------------------------------------------- //

} // clox
