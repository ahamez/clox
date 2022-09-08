#pragma once

#include <exception>
#include <iostream>
#include <iterator>
#include <type_traits>

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

struct Dispatch
{
  Chunk& chunk;
  VM& vm;
  Stack& stack;
  const Code::const_iterator current_ip;

  template<typename Op>
  void operator()(OpBinary<Op>)
  {
    const auto rhs = stack.pop();
    const auto lhs = stack.top();
    stack.top() = OpBinary<Op>{}(lhs, rhs);
  }

  void operator()(OpAdd)
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
  }

  void operator()(OpConstant op)
  {
    const auto value = chunk.code->get_constant(op.constant);
    stack.push(value);
  }

  void operator()(OpDefineGlobalVar op)
  {
    const auto var_value = stack.pop();
    vm.globals().insert_or_assign(op.global_variable_index, var_value);
  }

  void operator()(OpEqual)
  {
    const auto rhs = stack.pop();
    const auto lhs = stack.top();
    stack.top() = (rhs == lhs);
  }

  void operator()(OpFalse) { stack.push(false); }

  void operator()(OpGetGlobalVar op)
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
    }
  }

  void operator()(OpNegate) { stack.top() = -stack.top().as<double>(); }

  void operator()(OpNil) { stack.push(Nil{}); }

  void operator()(OpNot) { stack.push(stack.pop().falsey()); }

  void operator()(OpPop) { [[maybe_unused]] const auto _ = stack.pop(); }

  void operator()(OpPrint) { std::cout << stack.pop() << '\n'; }

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  [[noreturn]] Code::const_iterator operator()(OpReturn)
  {
    throw InterpretReturn{InterpretResultStatus::ok};
  }

  void operator()(OpSetGlobal op)
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
    }
  }

  void operator()(OpTrue) { stack.push(true); }
};

struct Interpret
{
  Chunk& chunk;
  VM& vm;
  Stack& stack;
  const Code::const_iterator current_ip;

  template<typename T>
  [[nodiscard]] Code::const_iterator operator()(const T& x) const
  {
    static constexpr auto op_modifies_current_ip =
      std::is_same<typename std::invoke_result<Dispatch, T>::type, Code::const_iterator>::value;

    if constexpr (op_modifies_current_ip)
    {
      return Dispatch{chunk, vm, stack, current_ip}(x);
    }
    else
    {
      Dispatch{chunk, vm, stack, current_ip}(x);
      return std::next(current_ip);
    }
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
