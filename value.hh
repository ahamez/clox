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
    : value_(Nil{})
  {
  }

  Value(Nil)
    : value_(Nil{})
  {
  }

  Value(double d)
    : value_{d}
  {
  }

  Value(bool b)
    : value_{b}
  {
  }

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
    struct falsey
    {
      bool operator()(bool b) const noexcept { return not b; }
      bool operator()(double) const noexcept { return false; }
      bool operator()(Nil) const noexcept { return false; }
    };

    return std::visit(falsey{}, value_);
  }

  friend std::ostream& operator<<(std::ostream& os, const Value& value)
  {
    // TODO: dispatch on real type to avoid calling std::boolalpha for double and Nil
    std::visit([&os](auto&& arg) { os << std::boolalpha << arg; }, value.value_);
    return os;
  }

  struct EqualityVisitor
  {
    bool operator()(bool lhs, bool rhs) const noexcept { return lhs == rhs; }
    bool operator()(double lhs, double rhs) const noexcept { return lhs == rhs; }
    bool operator()(Nil, Nil) const noexcept { return true; }
    bool operator()(auto, auto) const noexcept { return false; }
  };

  bool operator==(const Value& rhs) const
  {
    return std::visit(EqualityVisitor{}, value_, rhs.value_);
  }

  bool operator!=(const Value& rhs) const { return !(rhs == *this); }
};

// ---------------------------------------------------------------------------------------------- //

} // clox

template<>
struct fmt::formatter<clox::Value> : ostream_formatter
{};
