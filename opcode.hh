#pragma once

// ---------------------------------------------------------------------------------------------- //

#include <ostream>
#include <variant>

#include <fmt/core.h>

#include "value.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

template<typename Impl>
struct OpBinary
{
  std::string disassemble(const auto&) const { return std::string{Impl::sv}; }
  Value operator()(Value lhs, Value rhs) const
    noexcept(noexcept(std::declval<Impl>()(Value{}, Value{})))
  {
    return Impl{}(lhs, rhs);
  }
};

struct OpAddImpl
{
  static constexpr std::string_view sv = "OP_ADD";
  Value operator()(Value lhs, Value rhs) const noexcept
  {
    return lhs.as<double>() + rhs.as<double>();
  }
};
using OpAdd = OpBinary<OpAddImpl>;

struct OpDivideImpl
{
  static constexpr std::string_view sv = "OP_DIVIDE";
  Value operator()(Value lhs, Value rhs) const noexcept
  {
    return lhs.as<double>() / rhs.as<double>();
  }
};
using OpDivide = OpBinary<OpDivideImpl>;

struct OpMultiplyImpl
{
  static constexpr std::string_view sv = "OP_MULTIPLY";
  Value operator()(Value lhs, Value rhs) const noexcept
  {
    return lhs.as<double>() * rhs.as<double>();
  }
};
using OpMultiply = OpBinary<OpMultiplyImpl>;

struct OpSubtractImpl
{
  static constexpr std::string_view sv = "OP_SUBTRACT";
  Value operator()(Value lhs, Value rhs) const noexcept
  {
    return lhs.as<double>() - rhs.as<double>();
  }
};
using OpSubtract = OpBinary<OpSubtractImpl>;

// ---------------------------------------------------------------------------------------------- //

struct OpConstant
{
  std::size_t value_offset;

  std::string disassemble(const auto& chunk) const
  {
    return fmt::format("OP_CONSTANT {}", chunk.get_value(value_offset));
  }
};

// ---------------------------------------------------------------------------------------------- //

struct OpNegate
{
  std::string disassemble(const auto&) const { return "OP_NEGATE"; }
};

// ---------------------------------------------------------------------------------------------- //

struct OpReturn
{
  std::string disassemble(const auto&) const { return "OP_RETURN"; }
};

// ---------------------------------------------------------------------------------------------- //

using Opcode =
  std::variant<OpAdd, OpDivide, OpMultiply, OpSubtract, OpConstant, OpNegate, OpReturn>;

std::string
disassemble_opcode(const Opcode& opcode, const auto& chunk)
{
  return std::visit([&chunk](auto&& arg) { return arg.disassemble(chunk); }, opcode);
}

// ---------------------------------------------------------------------------------------------- //

} // clox
