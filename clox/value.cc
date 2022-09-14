#include "clox/value.hh"
#include "clox/detail/visitor.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

Value::Value(Nil)
  : value_(Nil{})
{}

Value::Value(double d)
  : value_{d}
{}

Value::Value(bool b)
  : value_{b}
{}

Value::Value(const ObjString* obj)
  : value_{obj}
{}

// ---------------------------------------------------------------------------------------------- //

bool
Value::falsey() const
{
  return std::visit(detail::visitor{[](bool b) { return not b; },
                                    [](double) { return false; },
                                    [](Nil) { return false; },
                                    [](const ObjString*) { return false; }},
                    value_);
}

// ---------------------------------------------------------------------------------------------- //

bool
operator==(const Value& lhs, const Value& rhs)
{
  return (lhs <=> rhs) == std::partial_ordering::equivalent;
}

std::partial_ordering
operator<=>(const Value& lhs, const Value& rhs)
{
  return std::visit(
    detail::visitor{
      [](bool lhs, bool rhs) -> std::partial_ordering
      { return lhs == rhs ? std::partial_ordering::equivalent : std::partial_ordering::unordered; },
      [](double lhs, double rhs) -> std::partial_ordering { return lhs <=> rhs; },
      [](Nil, Nil) -> std::partial_ordering { return std::partial_ordering::equivalent; },
      [](const ObjString* lhs, const ObjString* rhs) -> std::partial_ordering
      { return *lhs <=> *rhs; },
      [](const auto&, const auto&) -> std::partial_ordering
      { return std::partial_ordering::unordered; }},
    lhs.value_,
    rhs.value_);
}

// ---------------------------------------------------------------------------------------------- //

std::string
Value::type() const
{
  return std::visit(detail::visitor{[](bool) { return "boolean"; },
                                    [](double) { return "number"; },
                                    [](Nil) { return "nil"; },
                                    [](const ObjString*) { return "string"; }},
                    value_);
}

// ---------------------------------------------------------------------------------------------- //

std::ostream&
operator<<(std::ostream& os, const Value& value)
{
  std::visit(detail::visitor{[&os](bool arg) { os << std::boolalpha << arg; },
                             [&os](double arg) { os << arg; },
                             [&os](Nil arg) { os << arg; },
                             [&os](const ObjString* obj) { os << *obj; }},
             value.value_);
  return os;
}

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
