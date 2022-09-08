#pragma once

#include <exception>
#include <iosfwd>
#include <iterator>
#include <type_traits>

#include <fmt/core.h>

#include "clox/code.hh"
#include "clox/detail/stack.hh"
#include "clox/detail/visitor.hh"
#include "clox/nil.hh"
#include "clox/obj_string.hh"
#include "clox/opcode.hh"
#include "clox/vm.hh"

namespace clox::detail {

// ---------------------------------------------------------------------------------------------- //

struct InterpretReturn : public std::exception
{
  explicit InterpretReturn(VMResultStatus status)
    : InterpretReturn{status, {}}
  {}

  explicit InterpretReturn(VMResultStatus status, std::string msg)
    : status{status}
    , message{std::move(msg)}
  {}

  [[nodiscard]] const char* what() const noexcept override { return message.data(); }

  VMResultStatus status;
  std::string message;
};

// ---------------------------------------------------------------------------------------------- //

struct Dispatch
{
  Chunk& chunk;
  VM& vm;
  Stack& stack;
  std::ostream& os;
  const Code::const_iterator current_ip;

  template<typename Op>
  void operator()(OpBinary<Op>) const
  {
    const auto rhs = stack.pop();
    const auto lhs = stack.top();
    stack.top() = OpBinary<Op>{}(lhs, rhs);
  }

  void operator()(OpAdd) const
  {
    const auto rhs = stack.pop();
    const auto lhs = stack.top();

    const auto visitor = detail::visitor{
      [&](double lhs, double rhs) { stack.top() = OpAdd{}(lhs, rhs); },
      [&](const ObjString* lhs, const ObjString* rhs)
      { stack.top() = chunk.memory->make_string(lhs->str + rhs->str); },
      [](const auto&, const auto&) {
        throw InterpretReturn{VMResultStatus::runtime_error, "Operands must be numbers or strings"};
      }};

    std::visit(visitor, lhs.value(), rhs.value());
  }

  void operator()(OpConstant op) const
  {
    const auto value = chunk.code->get_constant(op.constant);
    stack.push(value);
  }

  void operator()(OpDefineGlobalVar op) const
  {
    const auto var_value = stack.pop();
    vm.globals().insert_or_assign(op.global_variable_index, var_value);
  }

  void operator()(OpEqual) const
  {
    const auto rhs = stack.pop();
    const auto lhs = stack.top();
    stack.top() = (rhs == lhs);
  }

  void operator()(OpFalse) const { stack.push(false); }

  void operator()(OpGetGlobalVar op) const
  {
    if (const auto search = vm.globals().find(op.global_variable_index);
        search == cend(vm.globals()))
    {
      throw InterpretReturn{
        VMResultStatus::runtime_error,
        fmt::format("Undefined variable {}",
                    chunk.memory->get_global_variable(op.global_variable_index))};
    }
    else
    {
      stack.push(search->second);
    }
  }

  void operator()(OpNegate) const { stack.top() = -stack.top().as<double>(); }

  void operator()(OpNil) const { stack.push(Nil{}); }

  void operator()(OpNot) const { stack.push(stack.pop().falsey()); }

  void operator()(OpPop) const { [[maybe_unused]] const auto _ = stack.pop(); }

  void operator()(OpPrint) const { os << stack.pop() << '\n'; }

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  [[noreturn]] Code::const_iterator operator()(OpReturn) const
  {
    throw InterpretReturn{VMResultStatus::ok};
  }

  void operator()(OpSetGlobal op) const
  {
    if (auto search = vm.globals().find(op.global_variable_index); search == cend(vm.globals()))
    {
      throw InterpretReturn{
        VMResultStatus::runtime_error,
        fmt::format("Undefined variable {}",
                    chunk.memory->get_global_variable(op.global_variable_index))};
    }
    else
    {
      search->second = stack.top();
    }
  }

  void operator()(OpTrue) const { stack.push(true); }
};

struct Interpret
{
  Chunk& chunk;
  VM& vm;
  detail::Stack& stack;
  std::ostream& os;
  const Code::const_iterator current_ip;

  template<typename T>
  [[nodiscard]] Code::const_iterator operator()(const T& x) const
  {
    static constexpr auto op_modifies_current_ip =
      std::is_same<typename std::invoke_result<Dispatch, T>::type, Code::const_iterator>::value;

    if constexpr (op_modifies_current_ip)
    {
      return Dispatch{chunk, vm, stack, os, current_ip}(x);
    }
    else
    {
      Dispatch{chunk, vm, stack, os, current_ip}(x);
      return std::next(current_ip);
    }
  }

  [[nodiscard]] Code::const_iterator visit() const
  {
    try
    {
      return std::visit(Interpret{chunk, vm, stack, os, current_ip}, *current_ip);
    }
    catch (const BadValueAccess& e)
    {
      const auto message =
        fmt::format("Bad type access: expected {}, got {}", e.expected_type, e.held_type);
      throw InterpretReturn{VMResultStatus::runtime_error, message};
    }
  }
};

// ---------------------------------------------------------------------------------------------- //

} // namespace clox::detail
