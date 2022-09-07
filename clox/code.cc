#include <iterator>

#include "clox/code.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

detail::ConstantIndex
Code::add_constant(Value v)
{
  const auto it = constants_.insert(constants_.end(), v);
  const auto index = static_cast<std::uint16_t>(it - constants_.cbegin());

  return detail::ConstantIndex{index};
}

Value
Code::get_constant(detail::ConstantIndex index) const
{
  return constants_[static_cast<std::uint16_t>(index)];
}

// ---------------------------------------------------------------------------------------------- //

Code::const_iterator
Code::cbegin() const noexcept
{
  return code_.cbegin();
}

// ---------------------------------------------------------------------------------------------- //

std::size_t
Code::code_offset(Code::const_iterator code_cit) const
{
  return std::distance(code_.cbegin(), code_cit);
}

// ---------------------------------------------------------------------------------------------- //

std::optional<std::size_t>
Code::line(Code::const_iterator code_cit) const
{
  return lines_[code_offset(code_cit)];
}

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
