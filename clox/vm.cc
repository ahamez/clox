#include <algorithm>
#include <exception>
#include <iostream>
#include <iterator>
#include <vector>

#include "clox/disassemble.hh"
#include "clox/vm.hh"

namespace clox {
namespace /* anonymous */ {

constexpr auto globals_reserve = 1024;

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
  VM& vm;
  Stack& stack;
  const Code::const_iterator current_ip;

  template<typename Op>
  Code::const_iterator operator()(OpBinary<Op>)
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

  Code::const_iterator operator()(OpAdd)
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

      stack.top() = chunk.code.memory().make_string(lhs_str + rhs_str);
      return std::next(current_ip);
    }
    else
    {
      throw InterpretReturn{InterpretResultStatus::runtime_error,
                            "Operands must be numbers or strings"};
    }
  }

  Code::const_iterator operator()(OpConstant op)
  {
    const auto value = chunk.code.get_constant(op.constant);
    stack.push(value);

    return std::next(current_ip);
  }

  Code::const_iterator operator()(OpDefineGlobalVar op)
  {
    const auto var_value = stack.pop();
    if (vm.globals().size() <= op.global_variable_index.index)
    {
      vm.globals().resize(op.global_variable_index.index + 1);
    }

    vm.globals()[op.global_variable_index.index] = var_value;

    return std::next(current_ip);
  }

  Code::const_iterator operator()(OpEqual)
  {
    const auto rhs = stack.pop();
    const auto lhs = stack.top();
    stack.top() = (rhs == lhs);

    return std::next(current_ip);
  }

  Code::const_iterator operator()(OpFalse)
  {
    stack.push(false);

    return std::next(current_ip);
  }

  Code::const_iterator operator()(OpGetGlobalVar op)
  {
    if (op.global_variable_index.index >= vm.globals().size())
    {
      throw InterpretReturn{
        InterpretResultStatus::runtime_error,
        fmt::format("Undefined variable {}",
                    chunk.code_cxt.get_global_variable(op.global_variable_index))};
    }

    stack.push(vm.globals()[op.global_variable_index.index]);

    return std::next(current_ip);
  }

  Code::const_iterator operator()(OpNegate)
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

  Code::const_iterator operator()(OpNil)
  {
    stack.push(Nil{});

    return std::next(current_ip);
  }

  Code::const_iterator operator()(OpNot)
  {
    stack.push(stack.pop().falsey());

    return std::next(current_ip);
  }

  Code::const_iterator operator()(OpPop)
  {
    [[maybe_unused]] const auto _ = stack.pop();

    return std::next(current_ip);
  }

  Code::const_iterator operator()(OpPrint)
  {
    std::cout << stack.pop() << '\n';

    return std::next(current_ip);
  }

  [[noreturn]] Code::const_iterator operator()(OpReturn)
  {
    throw InterpretReturn{InterpretResultStatus::ok};
  }

  Code::const_iterator operator()(OpTrue)
  {
    stack.push(true);

    return std::next(current_ip);
  }

  [[nodiscard]] Code::const_iterator visit() const
  {
    return std::visit(Interpret{chunk, vm, stack, current_ip}, *current_ip);
  }
};

template<VM::opt_disassemble Disassemble>
[[nodiscard]] InterpretResult
run(Chunk& chunk, VM& vm)
{
  auto current_ip = chunk.code.cbegin();
  auto stack = Stack{};

  try
  {
    while (true)
    {
      if constexpr (Disassemble == VM::opt_disassemble::yes)
      {
        std::cout << disassemble(current_ip, chunk) << '\n';
      }
      current_ip = Interpret{chunk, vm, stack, current_ip}.visit();
    }
  }
  catch (const InterpretReturn& r)
  {
    if (r.status != InterpretResultStatus::ok)
    {
      std::cerr << "line " << chunk.code.line(current_ip).value_or(0) << ": " << r.message << '\n';
    }
    return {r.status, std::move(stack)};
  }
}

} // namespace

// ---------------------------------------------------------------------------------------------- //

VM::VM(VM::opt_disassemble disassemble)
  : disassemble_{disassemble}
{
  globals_.reserve(globals_reserve);
}

// ---------------------------------------------------------------------------------------------- //

InterpretResult
VM::operator()(Chunk&& chunk)
{
  return (*this)(chunk);
}

InterpretResult
VM::operator()(Chunk& chunk)
{
  if (disassemble_ == opt_disassemble::yes)
  {
    return run<opt_disassemble::yes>(chunk, *this);
  }
  else
  {
    return run<opt_disassemble::no>(chunk, *this);
  }
}

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
