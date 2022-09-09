#pragma once

// ---------------------------------------------------------------------------------------------- //

#include <iostream>

#include <iosfwd>
#include <variant>

#include <fmt/core.h>

#include "clox/detail/index.hh"
#include "clox/value.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

template<typename Impl>
struct OpBinary
{
  [[nodiscard]] std::string disassemble(const auto&) const { return std::string{Impl::sv}; }

  Value operator()(Value lhs, Value rhs) const
    noexcept(noexcept(std::declval<Impl>()(Value{}, Value{})))
  {
    return Impl{}(lhs, rhs);
  }
};

// Arithmetic
struct OpAddImpl
{
  static constexpr std::string_view sv = "OP_ADD";
  Value operator()(Value lhs, Value rhs) const { return lhs.as<double>() + rhs.as<double>(); }
};
using OpAdd = OpBinary<OpAddImpl>;

struct OpDivideImpl
{
  static constexpr std::string_view sv = "OP_DIVIDE";
  Value operator()(Value lhs, Value rhs) const { return lhs.as<double>() / rhs.as<double>(); }
};
using OpDivide = OpBinary<OpDivideImpl>;

struct OpMultiplyImpl
{
  static constexpr std::string_view sv = "OP_MULTIPLY";
  Value operator()(Value lhs, Value rhs) const { return lhs.as<double>() * rhs.as<double>(); }
};
using OpMultiply = OpBinary<OpMultiplyImpl>;

struct OpSubtractImpl
{
  static constexpr std::string_view sv = "OP_SUBTRACT";
  Value operator()(Value lhs, Value rhs) const { return lhs.as<double>() - rhs.as<double>(); }
};
using OpSubtract = OpBinary<OpSubtractImpl>;

struct OpGreaterImpl
{
  static constexpr std::string_view sv = "OP_GREATER";
  Value operator()(Value lhs, Value rhs) const { return lhs.as<double>() > rhs.as<double>(); }
};
using OpGreater = OpBinary<OpGreaterImpl>;

struct OpLessImpl
{
  static constexpr std::string_view sv = "OP_LESS";
  Value operator()(Value lhs, Value rhs) const { return lhs.as<double>() < rhs.as<double>(); }
};
using OpLess = OpBinary<OpLessImpl>;

// ---------------------------------------------------------------------------------------------- //

struct OpConstant
{
  detail::ConstantIndex constant;

  [[nodiscard]] std::string disassemble(const auto& chunk) const
  {
    return fmt::format("OP_CONSTANT {}", chunk.code->get_constant(constant));
  }
};

// ---------------------------------------------------------------------------------------------- //

struct OpDefineGlobalVar
{
  detail::GlobalVariableIndex global_variable_index;

  [[nodiscard]] std::string disassemble(const auto& chunk) const
  {
    return fmt::format("OP_DEFINE_GLOBAL_VAR {}",
                       chunk.memory->get_global_variable(global_variable_index));
  }
};

// ---------------------------------------------------------------------------------------------- //

struct OpEqual
{
  [[nodiscard]] std::string disassemble(const auto&) const { return "OP_EQUAL"; }
};

// ---------------------------------------------------------------------------------------------- //

struct OpFalse
{
  [[nodiscard]] std::string disassemble(const auto&) const { return "OP_FALSE"; }
};

// ---------------------------------------------------------------------------------------------- //

struct OpGetGlobalVar
{
  detail::GlobalVariableIndex global_variable_index;

  [[nodiscard]] std::string disassemble(const auto& chunk) const
  {
    return fmt::format("OP_GET_GLOBAL_VAR {}",
                       chunk.memory->get_global_variable(global_variable_index));
  }
};

// ---------------------------------------------------------------------------------------------- //

struct OpNegate
{
  [[nodiscard]] std::string disassemble(const auto&) const { return "OP_NEGATE"; }
};

// ---------------------------------------------------------------------------------------------- //

struct OpNil
{
  [[nodiscard]] std::string disassemble(const auto&) const { return "OP_NIL"; }
};

// ---------------------------------------------------------------------------------------------- //

struct OpNot
{
  [[nodiscard]] std::string disassemble(const auto&) const { return "OP_NOT"; }
};

// ---------------------------------------------------------------------------------------------- //

template<std::size_t N>
struct OpPop
{
  [[nodiscard]] std::string disassemble(const auto&) const { return fmt::format("OP_POP<{}>", N); }
};

// ---------------------------------------------------------------------------------------------- //

struct OpPrint
{
  [[nodiscard]] std::string disassemble(const auto&) const { return "OP_PRINT"; }
};

// ---------------------------------------------------------------------------------------------- //

struct OpReturn
{
  [[nodiscard]] std::string disassemble(const auto&) const { return "OP_RETURN"; }
};

// ---------------------------------------------------------------------------------------------- //

struct OpSetGlobal
{
  detail::GlobalVariableIndex global_variable_index;

  [[nodiscard]] std::string disassemble(const auto&) const { return "OP_SET_GLOBAL"; }
};

// ---------------------------------------------------------------------------------------------- //

struct OpTrue
{
  [[nodiscard]] std::string disassemble(const auto&) const { return "OP_TRUE"; }
};

// ---------------------------------------------------------------------------------------------- //

using Opcode = std::variant<OpAdd,
                            OpConstant,
                            OpDefineGlobalVar,
                            OpDivide,
                            OpEqual,
                            OpFalse,
                            OpGetGlobalVar,
                            OpGreater,
                            OpLess,
                            OpMultiply,
                            OpNegate,
                            OpNil,
                            OpNot,
                            OpPop<1>,
                            OpPop<2>,
                            OpPrint,
                            OpSetGlobal,
                            OpSubtract,
                            OpReturn,
                            OpTrue>;

// ---------------------------------------------------------------------------------------------- //

[[nodiscard]] std::string
disassemble_opcode(const Opcode& opcode, const auto& chunk)
{
  return std::visit([&](auto&& arg) { return arg.disassemble(chunk); }, opcode);
}

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
