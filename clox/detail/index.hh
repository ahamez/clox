#pragma once

#include <cstdint>

#include <type_safe/strong_typedef.hpp>

namespace clox::detail {

// ---------------------------------------------------------------------------------------------- //

struct ConstantIndex : type_safe::strong_typedef<ConstantIndex, std::uint16_t>
{
  using strong_typedef::strong_typedef;
};

// ---------------------------------------------------------------------------------------------- //

struct GlobalVariableIndex
  : type_safe::strong_typedef<GlobalVariableIndex, std::uint16_t>
  , type_safe::strong_typedef_op::equality_comparison<GlobalVariableIndex>
  , type_safe::strong_typedef_op::relational_comparison<GlobalVariableIndex>
  , type_safe::strong_typedef_op::integer_arithmetic<GlobalVariableIndex>
{
  using strong_typedef::strong_typedef;
};

// ---------------------------------------------------------------------------------------------- //

} // namespace clox::detail

// ---------------------------------------------------------------------------------------------- //

namespace std {

template<>
struct hash<clox::detail::GlobalVariableIndex>
  : type_safe::hashable<clox::detail::GlobalVariableIndex>
{};

} // namespace std

// ---------------------------------------------------------------------------------------------- //
