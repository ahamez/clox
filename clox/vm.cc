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
      const auto lhs = stack.top();
      stack.top() = OpBinary<Op>{}(lhs, rhs);

      return std::next(current_ip);
    }
  }

  Chunk::code_const_iterator operator()(OpAdd)
  {
    if (stack.peek(0).is<double>() and stack.peek(1).is<double>())
    {
      const auto rhs = stack.pop();
      const auto lhs = stack.top();
      stack.top() = OpAdd{}(lhs, rhs);
      return std::next(current_ip);
    }
    else if (stack.peek(0).is<const ObjString*>() and stack.peek(1).is<const ObjString*>())
    {
      const auto rhs = stack.pop();
      const auto lhs = stack.top();

      const auto& lhs_str = lhs.as<const ObjString*>()->str;
      const auto& rhs_str = rhs.as<const ObjString*>()->str;

      stack.top() = chunk.memory().make_string(lhs_str + rhs_str);
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
    const auto value = chunk.get_constant(op.constant);
    stack.push(value);

    return std::next(current_ip);
  }

  Chunk::code_const_iterator operator()(OpEqual)
  {
    const auto rhs = stack.pop();
    const auto lhs = stack.top();
    stack.top() = (rhs == lhs);

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
      stack.top() = -stack.top().as<double>();

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

template<VM::opt_disassemble Disassemble>
[[nodiscard]] InterpretResult
run(Chunk& chunk)
{
  auto current_ip = chunk.code_cbegin();
  auto stack = Stack{};

  try
  {
    while (true)
    {
      if constexpr (Disassemble == VM::opt_disassemble::yes)
      {
        std::cout << chunk.disassemble(current_ip) << '\n';
      }
      current_ip = Interpret{chunk, stack, current_ip}.visit();
    }
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

} // namespace

// ---------------------------------------------------------------------------------------------- //

VM::VM(VM::opt_disassemble disassemble)
  : disassemble_{disassemble}
{}

// ---------------------------------------------------------------------------------------------- //

InterpretResult
VM::operator()(Chunk& chunk) const
{
  if (disassemble_ == opt_disassemble::yes)
  {
    return run<opt_disassemble::yes>(chunk);
  }
  else
  {
    return run<opt_disassemble::no>(chunk);
  }
}

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
