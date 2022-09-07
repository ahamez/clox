#pragma once

#include <exception>
#include <iostream>
#include <iterator>

#include <fmt/core.h>

#include "clox/code.hh"
#include "clox/detail/visitor.hh"
#include "clox/nil.hh"
#include "clox/obj_string.hh"
#include "clox/opcode.hh"
#include "clox/vm.hh"

namespace clox::detail {

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
    const auto rhs = stack.pop();
    const auto lhs = stack.top();
    stack.top() = OpBinary<Op>{}(lhs, rhs);

    return std::next(current_ip);
  }

  Code::const_iterator operator()(OpAdd)
  {
    const auto rhs = stack.pop();
    const auto lhs = stack.top();

    std::visit(detail::visitor{[&](double lhs, double rhs) { stack.top() = OpAdd{}(lhs, rhs); },
                               [&](const ObjString* lhs, const ObjString* rhs)
                               { stack.top() = chunk.memory->make_string(lhs->str + rhs->str); },
                               [](const auto&, const auto&)
                               {
                                 throw InterpretReturn{InterpretResultStatus::runtime_error,
                                                       "Operands must be numbers or strings"};
                               }},
               lhs.value(),
               rhs.value());

    return std::next(current_ip);
  }

  Code::const_iterator operator()(OpConstant op)
  {
    const auto value = chunk.code->get_constant(op.constant);
    stack.push(value);

    return std::next(current_ip);
  }

  Code::const_iterator operator()(OpDefineGlobalVar op)
  {
    const auto var_value = stack.pop();
    vm.globals().insert_or_assign(op.global_variable_index, var_value);

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
    if (const auto search = vm.globals().find(op.global_variable_index);
        search == cend(vm.globals()))
    {
      throw InterpretReturn{
        InterpretResultStatus::runtime_error,
        fmt::format("Undefined variable {}",
                    chunk.memory->get_global_variable(op.global_variable_index))};
    }
    else
    {
      stack.push(search->second);

      return std::next(current_ip);
    }
  }

  Code::const_iterator operator()(OpNegate)
  {
    stack.top() = -stack.top().as<double>();

    return std::next(current_ip);
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

  Code::const_iterator operator()(OpSetGlobal op)
  {
    if (auto search = vm.globals().find(op.global_variable_index); search == cend(vm.globals()))
    {
      throw InterpretReturn{
        InterpretResultStatus::runtime_error,
        fmt::format("Undefined variable {}",
                    chunk.memory->get_global_variable(op.global_variable_index))};
    }
    else
    {
      search->second = stack.top();

      return std::next(current_ip);
    }
  }

  Code::const_iterator operator()(OpTrue)
  {
    stack.push(true);

    return std::next(current_ip);
  }

  [[nodiscard]] Code::const_iterator visit() const
  {
    try
    {
      return std::visit(Interpret{chunk, vm, stack, current_ip}, *current_ip);
    }
    catch (const BadValueAccess& e)
    {
      const auto message =
        fmt::format("Bad type access: expected {}, got {}", e.expected_type, e.held_type);
      throw InterpretReturn{InterpretResultStatus::runtime_error, message};
    }
  }
};

// ---------------------------------------------------------------------------------------------- //

} // namespace clox::detail
