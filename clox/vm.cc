#include <algorithm>
#include <exception>
#include <iostream>
#include <iterator>
#include <vector>

#include "clox/vm.hh"

namespace clox {
namespace /* anonymous */ {

// ---------------------------------------------------------------------------------------------- //

struct InterpretReturn : public std::exception
{
  explicit InterpretReturn(InterpretResultStatus status)
    : InterpretReturn{status, {}}
  {}

  explicit InterpretReturn(InterpretResultStatus status, std::string msg)
    : status{status}
    , message{std::move(msg)}
  {}

  [[nodiscard]] const char* what() const noexcept override { return message.data(); }

  InterpretResultStatus status;
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
    if (not stack.peek(0).is<double>() or not stack.peek(1).is<double>())
    {
      throw InterpretReturn{InterpretResultStatus::runtime_error, "Operands must be numbers"};
    }
    else
    {
      const auto rhs = stack.pop();
      const auto lhs = stack.last();
      stack.last() = OpBinary<Op>{}(lhs, rhs);

      return std::next(current_ip);
    }
  }

  Chunk::code_const_iterator operator()(OpAdd)
  {
    if (stack.peek(0).is<double>() and stack.peek(1).is<double>())
    {
      const auto rhs = stack.pop();
      const auto lhs = stack.last();
      stack.last() = OpAdd{}(lhs, rhs);
      return std::next(current_ip);
    }
    else if (stack.peek(0).is<const ObjString*>() and stack.peek(1).is<const ObjString*>())
    {
      const auto rhs = stack.pop();
      const auto lhs = stack.last();

      const auto& lhs_str = lhs.as<const ObjString*>()->str;
      const auto& rhs_str = rhs.as<const ObjString*>()->str;

      stack.last() = chunk.memory().make_string(lhs_str + rhs_str);
      return std::next(current_ip);
    }
    else
    {
      throw InterpretReturn{InterpretResultStatus::runtime_error,
                            "Operands must be numbers or strings"};
    }
  }

  Chunk::code_const_iterator operator()(OpConstant op)
  {
    const auto value = chunk.get_value(op.value_offset);
    stack.push(value);

    return std::next(current_ip);
  }

  Chunk::code_const_iterator operator()(OpEqual)
  {
    const auto rhs = stack.pop();
    const auto lhs = stack.last();
    stack.last() = (rhs == lhs);

    return std::next(current_ip);
  }

  Chunk::code_const_iterator operator()(OpFalse)
  {
    stack.push(false);

    return std::next(current_ip);
  }

  Chunk::code_const_iterator operator()(OpNegate)
  {
    if (not stack.peek(0).is<double>())
    {
      throw InterpretReturn{InterpretResultStatus::runtime_error, "Operand must be a number"};
    }
    else
    {
      stack.last() = -stack.last().as<double>();

      return std::next(current_ip);
    }
  }

  Chunk::code_const_iterator operator()(OpNil)
  {
    stack.push(Nil{});

    return std::next(current_ip);
  }

  Chunk::code_const_iterator operator()(OpNot)
  {
    stack.push(stack.pop().falsey());

    return std::next(current_ip);
  }

  [[noreturn]] Chunk::code_const_iterator operator()(OpReturn)
  {
    throw InterpretReturn{InterpretResultStatus::ok};
  }

  Chunk::code_const_iterator operator()(OpTrue)
  {
    stack.push(true);

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
    if (r.status != InterpretResultStatus::ok)
    {
      std::cerr << "line " << chunk.line(current_ip).value_or(0) << ": " << r.message << '\n';
    }
    return {r.status, std::move(stack)};
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
