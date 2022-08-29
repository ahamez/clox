#pragma once

#include <iosfwd>
#include <variant>

#include <fmt/ostream.h>

#include "clox/nil.hh"
#include "clox/obj_string.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

class Value
{
private:
  using type = std::variant<double, bool, Nil, const ObjString*>;
  type value_;

public:
  Value();
  Value(Nil);
  Value(double);
  Value(bool);
  Value(const ObjString*);

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

  [[nodiscard]] bool falsey() const;

  bool operator==(const Value& rhs) const;
  bool operator!=(const Value& rhs) const;

  friend std::ostream& operator<<(std::ostream& os, const Value& value);
};

// ---------------------------------------------------------------------------------------------- //

} // clox

template<>
struct fmt::formatter<clox::Value> : ostream_formatter
{};
