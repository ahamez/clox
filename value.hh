#pragma once

#include <ostream>
#include <variant>

#include <fmt/ostream.h>

namespace clox {

// ---------------------------------------------------------------------------------------------- //

struct Nil
{
  friend std::ostream& operator<<(std::ostream& os, const Nil& nil) { return os << "nil"; }
};

struct Value
{
  using type = std::variant<double, bool, Nil>;
  type value_;

  Value()
    : value_()
  {
  }

  Value(double d)
    : value_{d}
  {
  }

  template<typename T>
  T as() const
  {
    return std::get<T>(value_);
  }

  template<typename T>
  T is() const
  {
    return std::holds_alternative<T>(value_);
  }

  friend std::ostream& operator<<(std::ostream& os, const Value& value)
  {
    std::visit([&os](auto&& arg) { os << arg; }, value.value_);
    return os;
  }
};

// ---------------------------------------------------------------------------------------------- //

} // clox

template<>
struct fmt::formatter<clox::Value> : ostream_formatter
{};
