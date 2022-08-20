#include <algorithm>
#include <exception>
#include <iostream>
#include <iterator>
#include <ostream>
#include <vector>

#include "vm.hh"

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
    , message{msg}
  {}

  [[nodiscard]] const char* what() const noexcept override { return message.data(); }

  InterpretResult result;
  std::string message;
};

// ---------------------------------------------------------------------------------------------- //

struct Interpret
{
  const Chunk& chunk;
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

  [[nodiscard]] Chunk::code_const_iterator visit(std::ostream& os) const
  {
    os << chunk.disassemble(current_ip) << '\n';
    return visit();
  }

  [[nodiscard]] Chunk::code_const_iterator visit() const
  {
    return std::visit(Interpret{chunk, stack, current_ip}, *current_ip);
  }
};

// NOLINTBEGIN(misc-no-recursion)
[[nodiscard]] InterpretResult
run(const Chunk& chunk, Stack& stack, Chunk::code_const_iterator current_ip, std::ostream& os)
{
  try
  {
    const auto next_ip = Interpret{chunk, stack, current_ip}.visit(os);
    return run(chunk, stack, next_ip, os);
  }
  catch (const InterpretReturn& r)
  {
    return r.result;
  }
}

[[nodiscard]] InterpretResult
run(const Chunk& chunk, Stack& stack, Chunk::code_const_iterator current_ip)
{
  try
  {
    const auto next_ip = Interpret{chunk, stack, current_ip}.visit();
    return run(chunk, stack, next_ip);
  }
  catch (const InterpretReturn& r)
  {
    if (r.result != InterpretResult::ok)
    {
      const auto offset = std::distance(chunk.code(), current_ip);
      std::cerr << "line " << chunk.lines()[offset].value_or(0) << ": " << r.message << '\n';
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
VM::operator()(const Chunk& chunk) const
{
  auto stack = Stack{};
  stack.reserve(1024);

  auto ip = chunk.code();

  if (disassemble_ == opt_disassemble::yes)
  {
    return run(chunk, stack, ip, std::cout);
  }
  else
  {
    return run(chunk, stack, ip);
  }
}

// ---------------------------------------------------------------------------------------------- //

} // clox
