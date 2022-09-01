#include "clox/obj_string.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

ObjString::ObjString(std::string str)
  : str{std::move(str)}
{}

// ---------------------------------------------------------------------------------------------- //

bool
ObjString::operator==(const ObjString& rhs) const noexcept
{
  return str == rhs.str;
}

bool
ObjString::operator!=(const ObjString& rhs) const noexcept
{
  return !(rhs == *this);
}

std::ostream&
operator<<(std::ostream& os, const ObjString& string)
{
  return os << string.str;
}

// ---------------------------------------------------------------------------------------------- //

} // namespace clox

// ---------------------------------------------------------------------------------------------- //

std::size_t
std::hash<clox::ObjString>::operator()(const clox::ObjString& obj) const noexcept
{
  return std::hash<std::string>{}(obj.str);
}

// ---------------------------------------------------------------------------------------------- //
