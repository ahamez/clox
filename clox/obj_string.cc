#include "clox/obj_string.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

ObjString::ObjString(std::string str)
  : str{std::move(str)}
{}

// ---------------------------------------------------------------------------------------------- //

std::strong_ordering
operator<=>(const ObjString& lhs, const ObjString& rhs) noexcept
{
  // The following _should_ work, but it doesn't compile 🤔.
  // return lhs.str <=> rhs.str;
  return lhs.str == rhs.str  ? std::strong_ordering::equal
         : lhs.str < rhs.str ? std::strong_ordering::less
                             : std::strong_ordering::greater;
}

bool
operator==(const ObjString& lhs, const ObjString& rhs) noexcept
{
  return lhs.str == rhs.str;
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
