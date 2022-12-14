#pragma once

#include <compare>
#include <iosfwd>
#include <variant>

#include <fmt/ostream.h>

#include "clox/nil.hh"
#include "clox/obj_string.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

struct BadValueAccess : public std::exception
{
  BadValueAccess(std::string expected_type, std::string held_type)
    : expected_type{std::move(expected_type)}
    , held_type{std::move(held_type)}
  {}

  [[nodiscard]] const char* what() const noexcept override { return "Bad type"; }

  std::string expected_type;
  std::string held_type;
};

// ---------------------------------------------------------------------------------------------- //

class Value
{
private:
  std::variant<double, bool, Nil, const ObjString*> value_{Nil{}};

public:
  Value() = default;
  // NOLINTBEGIN(hicpp-explicit-conversions)
  Value(Nil);
  Value(double);
  Value(bool);
  Value(const ObjString*);
  // NOLINTEND(hicpp-explicit-conversions)

  template<typename T>
  [[nodiscard]] T as() const
  {
    try
    {
      return std::get<T>(value_);
    }
    catch (const std::bad_variant_access&)
    {
      throw BadValueAccess{Value{T{}}.type(), type()};
    }
  }

  template<typename T>
  [[nodiscard]] bool is() const
  {
    return std::holds_alternative<T>(value_);
  }

  [[nodiscard]] bool falsey() const;

  friend bool operator==(const Value&, const Value&);
  friend std::partial_ordering operator<=>(const Value&, const Value&);

  friend std::ostream& operator<<(std::ostream&, const Value&);

  [[nodiscard]] std::string type() const;
  [[nodiscard]] const auto& value() const noexcept { return value_; }
};

// ---------------------------------------------------------------------------------------------- //

} // namespace clox

template<>
struct fmt::formatter<clox::Value> : ostream_formatter
{};
