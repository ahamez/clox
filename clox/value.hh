#pragma once

#include <iosfwd>
#include <variant>

#include <fmt/ostream.h>

#include "clox/nil.hh"
#include "clox/visitor.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

struct Value
{
  using type = std::variant<double, bool, Nil>;
  type value_;

  Value()
    : value_(Nil{})
  {}

  Value(Nil)
    : value_(Nil{})
  {}

  Value(double d)
    : value_{d}
  {}

  Value(bool b)
    : value_{b}
  {}

  template<typename T>
  [[nodiscard]] T as() const
  {
    return std::get<T>(value_);
  }

  template<typename T>
  [[nodiscard]] bool is() const
  {
    return std::holds_alternative<T>(value_);
  }

  [[nodiscard]] bool falsey() const noexcept
  {
    return std::visit(visitor{[](bool b) { return not b; },
                              [](double) { return false; },
                              [](Nil) { return false; },
                      value_);
  }

  friend std::ostream& operator<<(std::ostream& os, const Value& value)
  {
    std::visit(visitor{[&os](bool arg) { os << std::boolalpha << arg; },
                       [&os](double arg) { os << arg; },
                       [&os](Nil arg) { os << arg; },
               value.value_);
    return os;
  }

  bool operator==(const Value& rhs) const
  {
    return std::visit(visitor{[](bool lhs, bool rhs) { return lhs == rhs; },
                              [](double lhs, double rhs) { return lhs == rhs; },
                              [](Nil, Nil) { return true; },
                              [](const auto&, const auto&) { return false; }},
                      value_,
                      rhs.value_);
  }

  bool operator!=(const Value& rhs) const { return !(rhs == *this); }
};

// ---------------------------------------------------------------------------------------------- //

} // clox

template<>
struct fmt::formatter<clox::Value> : ostream_formatter
{};
